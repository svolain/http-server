/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 13:13:54 by vsavolai          #+#    #+#             */
/*   Updated: 2024/11/08 12:43:09 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"
#include "Logger.hpp"
#include "ClientConnection.hpp"
#include "CgiConnection.hpp"



HttpParser::HttpParser(ClientConnection& client) : client_(client) {}

bool HttpParser::ParseHeader(const std::string& request) {
  std::istringstream  request_stream(request);

  if (!ParseStartLine(request_stream))
    return false;
  if (!ParseHeaderFields(request_stream))
    return false;

  std::streampos current = request_stream.tellg();
  request_stream.seekg(0, std::ios::end);
  size_t stream_size = request_stream.tellg() - current;

  if (method_ == "POST") {
    if (!CheckPostHeaders())
      return false;
    request_stream.seekg(current);
    request_body_.resize(stream_size);
    request_stream.read(request_body_.data(), stream_size);
    return true;
  }
  if (stream_size == 0)
    return true;
  else {
    client_.status_ = "400";
    logError("Requset contains body.");
    return false;
  }
}

bool  HttpParser::HandleRequest() {
  const LocationMap& locations = client_.vhost_->getLocations();
  const Location* loc_ptr = FindLocation(locations);

  if (loc_ptr == nullptr) {
    logError("Location not found");
    client_.status_ = "404";
    return false;
  }

  const Location& loc = *loc_ptr;
  if (loc.methods_.find(method_) == std::string::npos) {
    logError("Method not allowed");
    client_.status_ = "405";
    return false;
  }

  if (!loc.redirection_.first.empty()) {
    client_.status_ = loc.redirection_.first;
    client_.additional_headers_["Location:"] = loc.redirection_.second;
    client_.additional_headers_["Content-Length:"] = "0";
    client_.stage_ = ClientConnection::Stage::kResponse;
    return true;
  }

  index_ = loc.index_;
  uploads_ = loc.upload_;
  HandleCookies();
  if (method_ == "GET" && !HandleGet(loc.autoindex_))
    return false;
  else if (method_ == "DELETE" && !HandleDeleteRequest())
    return false;
  else if (method_ == "POST") {
    logDebug(request_body_.data());
    if (!IsBodySizeValid())
      return false;

    if (!request_body_.empty() && content_length_ == request_body_.size()) {
      return HandlePostRequest(request_body_);
    } else {
        client_.stage_ = ClientConnection::Stage::kBody;
    }
  }
  return true;
}

const Location* HttpParser::FindLocation(const LocationMap& locations) {
  const Location* out = nullptr;
  size_t        max_length = 0;
  for (const auto& pair : locations) {
    if (request_target_.find(pair.first) == 0 && 
        pair.first.length() > max_length) {
      out = &(pair.second);
      max_length =  pair.first.length();
    }
  }
  if (out != nullptr) {
    request_target_ = out->root_ + request_target_.substr(max_length);
    logDebug("Updated request_target:", request_target_);
  }
  return out;
}

bool HttpParser::WriteBody(std::vector<char>& buffer, int bytesIn) {
  std::string eoc = "0\r\n\r\n";
  if (is_chunked_) {
    if (bytesIn > 5 || !std::equal(buffer.begin(), buffer.end(), eoc.begin())) {
      logDebug("bytesIn == MAXBYTES, more data to receieve");
      AppendBody(buffer, bytesIn);
      if (!is_chunked_ &&
          (request_body_.size() > content_length_)) {
        logError("Error: Content Too Large");
        client_.status_ = "413";
        return false;
      }
      return true;
    }
    UnChunkBody(request_body_);
    content_length_ = request_body_.size();
  } else {
    AppendBody(buffer, bytesIn);
    if (bytesIn == MAXBYTES)
      return true;
  }
  if (!HandlePostRequest(request_body_))
    return false;

  return true;
}

bool HttpParser::IsBodySizeValid() {
  if (request_body_.size() > client_.vhost_->getMaxBodySize()) {
    logError("Content body is too large");
    client_.status_ = "413";
    return false;
  }
  return true;
}

void HttpParser::ResetParser() {
  content_length_ = 0;
  method_.clear();
  request_target_.clear();
  query_string_.clear();
  file_list_.clear();
  index_.clear();
  session_id_.clear();
  request_body_.clear();
  headers_.clear();
  session_store_.clear();
  is_chunked_ = false;
  content_type_.clear();
  uploads_.clear();
}

std::string HttpParser::getHost() {
  return headers_["Host"];
}

std::string HttpParser::getRequestTarget() const {
  return request_target_;
}

bool HttpParser::ParseStartLine(std::istringstream& request_stream) {
  std::string line;
  std::string http_version;
  std::getline(request_stream, line);
  std::istringstream line_stream(line);
  line_stream >> method_ >> request_target_ >> http_version;
  std::getline(line_stream, line);

  if (method_.empty() || request_target_.empty()
      || http_version.empty() || line != "\r") {
    logError("Bad request 400");
    client_.status_ = "400";
    return false;
  }

  static std::array<std::string, 3> allowed_methods = {"GET", "POST", "DELETE"};
  if (std::find(allowed_methods.begin(), allowed_methods.end(), method_) ==
      allowed_methods.end()) {
    logError("Not supported method requested");
    client_.status_ = "501";
    return false;
  }

  if (request_target_[0] != '/') {
    logError("Bad request 400");
    client_.status_ = "400";
    return false;
  }

  if (http_version != "HTTP/1.1") {
    logError("HTTP Version Not Supported 505");
    client_.status_ = "505";
    return false;
  }

  size_t query_position = request_target_.find("?");
  if (query_position != std::string::npos) {
    query_string_ = request_target_.substr(query_position + 1);
    request_target_ = request_target_.substr(0, query_position);
  }
  return true;
}

bool HttpParser::ParseHeaderFields(std::istringstream& request_stream) {
  std::string line;
  bool        bad_request = false;
  while (std::getline(request_stream, line) && line != "\r") {
    size_t delim = line.find(":");
    if (delim == std::string::npos || line.back() != '\r') {
      bad_request = true;
      continue;
    }
    line.pop_back();
    std::string header = line.substr(0, delim);
    std::string header_value = line.substr(delim + 1);
    headers_[header] = header_value;
  }

  if (line != "\r") {
    logError("Request header fields too large");
    client_.status_ = "431";
    return false;
  }

    if (bad_request) {
      logError("Wrong header line format");
      client_.status_ = "400";
      return false; 
    }

  if (!headers_.contains("Host")) {
    logError("Bad request 400");
    client_.status_ = "400";
    return false;
  }

  if (headers_.contains("Cookie")) {
        std::string cookie_header = headers_["Cookie"];
        ParseCookies(cookie_header);
  }

  return true;
}

bool HttpParser::CheckPostHeaders() {
  {
    auto it = headers_.find("Content-Type");
    if (it == headers_.end()) {
      logError("Content-Type missing for request body");
      client_.status_ = "400";
      return false;
    }
    content_type_ =it->second;
  }
  auto it = headers_.find("transfer-encoding");
  is_chunked_ = (it != headers_.end() && it->second == "chunked");
  if (!is_chunked_) {
    auto it = headers_.find("Content-Length");
    if (it == headers_.end()) {
      logError("Content-lenght missing for request body");
      client_.status_ = "411";
      return false;
    } else {
      std::string& content_length_str = it->second;
      try {
        size_t pos;
        int content_length = std::stoi(content_length_str, &pos);
        if (pos != content_length_str.size() || content_length < 0)
          throw std::invalid_argument("Invalid argument");
        content_length_ = content_length;
      } catch (const std::invalid_argument& e) {
        logError("Invalid Content-Length");
        client_.status_ = "400";
        return false;
      } catch (const std::out_of_range& e) {
        logError("Content-Length out of range");
        client_.status_ = "413";
        return false;
      }
    }
  }
  return true;
}

void HttpParser::ParseCookies(const std::string& cookie_header) {
  std::istringstream cookie_stream(cookie_header);
  std::string cookie_pair;

  while (std::getline(cookie_stream, cookie_pair, ';')) {
    size_t delim = cookie_pair.find('=');
    if (delim != std::string::npos) {
      std::string name = cookie_pair.substr(0, delim);
      std::string value = cookie_pair.substr(delim + 1);
      name = TrimWhitespace(name);
      value = TrimWhitespace(value);
      session_store_[name] = value;
    }
  }
}

std::string HttpParser::TrimWhitespace(const std::string& str) {
  size_t first = str.find_first_not_of(' ');
  size_t last = str.find_last_not_of(' ');
  return str.substr(first, (last - first + 1));
}

static std::string CreateSessionID() {
  std::ostringstream ss;
  ss << std::hex << std::random_device{}();
  return ss.str();
}


void HttpParser::HandleCookies() {
  if (session_store_.find("session_id") != session_store_.end()) {
    session_id_ = session_store_["session_id"];
    logDebug("Returning User with session_id: ", session_id_);
  } else {
    session_id_ = CreateSessionID();
    logDebug("New User. Generated session_id: ", session_id_);
    client_.additional_headers_["Set-Cookie: session_id="] = session_id_;
  }
}

bool HttpParser::UnChunkBody(std::vector<char>& buf) {
  std::size_t readIndex = 0;
  std::size_t writeIndex = 0;

  while(readIndex < buf.size()) {
    std::size_t chunkSizeStart = readIndex;

    while (readIndex < buf.size() && !(buf[readIndex] == '\r'
            && buf[readIndex + 1] == '\n')) {
      readIndex++;
    }

    if (readIndex >= buf.size()) {
      logError("UnChunkBody: \\r\\n missing");
      client_.status_ = "400";
      return false;
    }

    std::string chunkSizeStr(buf.begin() + chunkSizeStart,
                             buf.begin() + readIndex);
    std::size_t chunkSize;
    std::stringstream ss;
    ss << std::hex << chunkSizeStr;
    ss >> chunkSize;

    readIndex += 2;

    if (chunkSize == 0)
      break;


    if (readIndex + chunkSize > buf.size()) {
      logError("UnChunkBody: empty line missing");
      client_.status_ = "400";
      return false;
    }

    for (std::size_t i = 0; i < chunkSize; ++i)
      buf[writeIndex++] = buf[readIndex++];

    if (buf[readIndex] == '\r' && buf[readIndex + 1] == '\n') {
      readIndex += 2;
    } else {
      client_.status_ = "400";
      logError("UnChunkBody: \\r\\n missing");
      client_.status_ = "400";
      return false;
    }
  }
  buf.resize(writeIndex);
  return true;
}

void HttpParser::AppendBody(std::vector<char> buffer, int bytesIn) {
  request_body_.insert(request_body_.end(), buffer.begin(),
                       buffer.begin() + bytesIn);
}

bool HttpParser::HandlePostRequest(std::vector<char>& request_body) {
  std::string filename = "/tmp/webserv/post_" + std::to_string(client_.fd_);
  if (OpenFile(filename))
    return false;
  std::remove(filename.c_str());

  if (request_target_.ends_with(".cgi") ||
      request_target_.ends_with(".py") || request_target_.ends_with(".php") ) {
    if (!CheckValidPath()) {
      client_.file_.close();
      return false;
    }
    client_.file_ << request_body.data() << std::flush;
    client_.file_.seekg(0);
    pid_t pid = CgiConnection::CreateCgiConnection(client_);
    if (pid == -1) {
      client_.status_ = "500";
      return false;
    }
    client_.stage_ = ClientConnection::Stage::kCgi;
    return true;
  }

  if (content_type_.find("multipart/form-data") != std::string::npos) {
    logDebug("Handling multipart form data");
    if (!HandleMultipartFormData(request_body, content_type_)) {
      return false;
    }
  } else {
    static const std::map<std::string, std::string> types = getContTypeMap();
    std::string extension;
    auto it = types.find(content_type_);
    if (it == types.end())
      extension =  ".txt";
    else 
      extension = it->second;
    std::ofstream outFile(uploads_ + "upload" + std::to_string(client_.fd_) + extension, std::ios::binary);
    if (outFile.is_open()) {
      outFile << request_body.data();
      outFile.close();
      logDebug("File ", filename, " saved successfully");
    } else {
      logError("Failed to save file ");
      client_.status_ = "500";
      client_.file_.close();
      return false;  
    }
  }
  std::string htmlStr = InjectFileListIntoHtml(request_target_ + "/" + index_);
  client_.file_ << htmlStr;
  client_.stage_ = ClientConnection::Stage::kResponse;
  client_.file_.seekg(0);
  return true;
}

bool HttpParser::HandleMultipartFormData(const std::vector<char> &body,
                                         const std::string &contentType) {
  size_t boundaryPosition = contentType.find("boundary=");
  if (boundaryPosition == std::string::npos) {
    client_.status_ = "500";
    return false;
  }

  std::string boundary = "--" + contentType.substr(boundaryPosition + 9);
  std::string fullBoundary = "\r\n" + boundary;

  std::vector<char> boundaryVec(boundary.begin(), boundary.end());
  std::vector<char> fullBoundaryVec = {'\r', '\n'};
  fullBoundaryVec.insert(fullBoundaryVec.end(), boundaryVec.begin(),
                         boundaryVec.end());

  size_t boundaryLength = boundaryVec.size();
  size_t fullBoundaryLength = fullBoundaryVec.size();

  auto pos = body.begin();
  bool firstBoundary = true;
  while (true) {
    auto boundaryStart = body.end();
    if (firstBoundary) {
      boundaryStart = std::search(pos, body.end(), boundaryVec.begin(),
                                  boundaryVec.end());
    } else {
      boundaryStart = std::search(pos, body.end(), fullBoundaryVec.begin(),
                                  fullBoundaryVec.end());
    }

    if (boundaryStart == body.end())
      break;

    pos = boundaryStart + (firstBoundary ? boundaryLength : fullBoundaryLength);
    firstBoundary = false;
    auto nextBoundaryStart = std::search(pos, body.end(),
                                         fullBoundaryVec.begin(),
                                         fullBoundaryVec.end());
    if (nextBoundaryStart == body.end())
      break;
    std::vector<char> bodyPart(pos, nextBoundaryStart);
    if (!ParseMultiPartData(bodyPart))
      return false;
    pos = nextBoundaryStart;
  }
  return true;
}

bool HttpParser::ParseMultiPartData(std::vector<char> &bodyPart) {
  std::vector<char> crlf = {'\r', '\n', '\r', '\n'};
  auto headerEndIt = std::search(bodyPart.begin(), bodyPart.end(),
                                 crlf.begin(), crlf.end());
  if (headerEndIt == bodyPart.end()) {
    logError("Invalid multi part format");
    return false;
  }

  std::vector<char> headers(bodyPart.begin(), headerEndIt);
  std::vector<char> content(headerEndIt + 4, bodyPart.end());

  std::string headersStr(headers.begin(), headers.end());

  if (headersStr.find("Content-Disposition") != std::string::npos) {
    size_t posFilename = headersStr.find("filename=");
    size_t posName = headersStr.find("name=");

    if (posFilename != std::string::npos) {
      size_t posStart = headersStr.find('"', posFilename) + 1;
      size_t posEnd = headersStr.find('"', posStart);
      std::string filename = headersStr.substr(posStart, posEnd - posStart);
      logDebug("File upload: ", filename);
      std::ofstream outFile(uploads_ + filename, std::ios::binary);
      if (outFile.is_open()) {
        outFile.write(content.data(), content.size());
        outFile.close();
        logDebug("File ", filename, " saved successfully");
      } else {
        logError("Failed to save file ");
        client_.status_ = "500";
        client_.file_.close();
        return false;
      }
    } else if (posName != std::string::npos) {
      size_t posStart = headersStr.find('"', posName) + 1;
      size_t posEnd = headersStr.find('"', posStart);
      std::string fieldName = headersStr.substr(posStart, posEnd - posStart);
      std::string contentStr(content.begin(), content.end());
      logDebug("Form field: ", fieldName, " = ", contentStr);
    }
  }
  return true;
}

std::string UrlDecode( std::string& query) {
  std::string decoded;
    for (size_t i = 0; i < query.length(); i++) {
      if (query[i] == '%' && i + 2 < query.length()) {
        std::string hexValue = query.substr(i + 1, 2);
        char decodedChar = static_cast<char>(std::stoi(hexValue, nullptr, 16));
        decoded += decodedChar;
        i += 2;
      } else if (query[i] == '+') {
        decoded += ' ';
      } else {
        decoded += query[i];
      }
    }
    return decoded;
}

bool HttpParser::HandleDeleteRequest() {
  size_t pos = request_target_.find("/");
  std::string file;
  if (pos != std::string::npos)
    file = request_target_.substr(pos);
  else
    return false;
  std::string path = uploads_ + file;
  logDebug("Handling DELETE request for: ", path);
  if (path.find("..") != std::string::npos)
    return false;

  if (std::filesystem::exists(path)) {
    if (std::remove(path.c_str()) == 0) {
      logDebug("File deleted successfully");
      client_.status_ = "200";
    } else {
      logError("Failed to delete file: " + path);
      client_.status_ = "500";
      return false;
    }
  } else {
    logError("File not found: ", path);
    client_.status_ = "404";
    return false;
  }
  std::string clientFd = std::to_string(client_.fd_);
  std::string filename = "/tmp/webserv/delete_list"  + clientFd;
  std::fstream& outFile = client_.file_;
  if (OpenFile(filename))
    return false;
  std::remove(filename.c_str());
  std::string root = request_target_.substr(0, pos);
  std::string htmlStr = InjectFileListIntoHtml(root + "/" + index_);
  outFile << htmlStr;
  client_.stage_ = ClientConnection::Stage::kResponse;
  outFile.seekg(0);
  return true;
}

void HttpParser::GenerateFileListHtml() {
  file_list_ = "<ul>";
  try {
    for (const auto &entry : std::filesystem::directory_iterator(uploads_)) {
      std::string filename = entry.path().filename().string();
      file_list_ += "<li>";
      file_list_ += "<span><a href=\"/" + uploads_ + filename + "\">/" + uploads_ + filename + "</a></span>";
      file_list_ += "<button onclick=\"deleteFile('" + filename + "')\">Delete</button>";
      file_list_ += "</li>";
    }
  } catch (const std::filesystem::filesystem_error& e) {
    file_list_ += "<li>Error reading directory: " + std::string(e.what()) + "</li>";
  } catch (const std::exception& e) {
    file_list_ += "<li>Unexpected error: " + std::string(e.what()) + "</li>";
  }
  file_list_ += "</ul>";
}

bool HttpParser::CheckValidPath() {
  try {
    if (std::filesystem::exists(request_target_)) {
      if (std::filesystem::is_directory(request_target_)) {
        if (request_target_.back() != '/')
          request_target_ += "/" + index_;
        else
          request_target_ += index_;
        return true; //The path is a directory
      } else if (std::filesystem::is_regular_file(request_target_)) {
        return true; //The path is a file
      }
      } else {
        logError("The path does not exist: ", request_target_);
        client_.status_ = "404";
        return false;
    }
  } catch (const std::filesystem::filesystem_error& e) {
    logError("Filesystem error: ");
    std::cerr << e.what() << std::endl;
    client_.status_ = "500";
    return false;
  } catch (const std::exception& e) {
    logError("Unexpected error: "); 
    std::cerr << e.what() << std::endl;
    client_.status_ = "500";
    return false;
  }
  return true;
}

bool HttpParser::CreateDirListing(std::string& directory) {
  std::string filename = "/tmp/webserv/dir_list"  + std::to_string(client_.fd_);
  std::fstream& outFile = client_.file_;
  if (OpenFile(filename)) 
    return false;
  std::remove(filename.c_str());

  outFile << "<html><body><h1>Index of " << directory << "</h1><ul>";
  try {
    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
      std::string name = entry.path().filename().string();
      if (std::filesystem::is_directory(entry.path())) {
        outFile << "<li>" << name << "</li>";
      } else {
        outFile << "<li>" << name << "</li>";
      }
    }
  } catch (const std::filesystem::filesystem_error& e) {
    outFile << "<h1>Directory not found</h1>";
    logError("Error accessing directory: ", directory);
    client_.status_ = "500";
    return false;
  }
  outFile<< "</ul></body></html>";
  logDebug("Directory listing created");
  outFile.seekg(0);
  return true;
}

int HttpParser::OpenFile(std::string& filename) {
  logDebug(filename);
  std::fstream& file = client_.file_;
  file.open(filename, std::fstream::in | std::fstream::out| std::fstream::app | std::ios::binary);
  if (!file.is_open()) {
    client_.status_ = "500";
    return 1;
  }
  return 0;
}

static bool existIndex(std::string& target, std::string& index) {
  if (index.size() == 0)
    return false;
  std::string path = target + index;
  if (access(path.c_str(), R_OK) == -1)
    return false;
  return true;
}

bool HttpParser::HandleGet(bool autoIndex) {
  if (autoIndex && (!existIndex(request_target_, index_)) && request_target_.back() == '/') {
    if (!CreateDirListing(request_target_))
      return false;
    client_.stage_ = ClientConnection::Stage::kResponse;
    return true;
  } else if (!CheckValidPath())
    return false;
  if (request_target_.ends_with(".cgi") ||
      request_target_.ends_with(".py") || request_target_.ends_with(".php") ) {
    pid_t pid = CgiConnection::CreateCgiConnection(client_);
    if (pid == -1) {
      client_.status_ = "500";
      return false;
    }
    client_.stage_ = ClientConnection::Stage::kCgi;
    return true;
  }
  if (OpenFile(request_target_))
    return false;
  client_.stage_ = ClientConnection::Stage::kResponse;
  return true;
}

std::string HttpParser::InjectFileListIntoHtml(const std::string& html_path) {
  std::ifstream html_file(html_path);
  if (!html_file.is_open()) {
      return "<html><body><h1>Error: Unable to open HTML file.</h1></body></html>";
  }
  std::string html_content((std::istreambuf_iterator<char>(html_file)),
                           std::istreambuf_iterator<char>());
  GenerateFileListHtml();
  std::size_t placeholder_pos = html_content.find("<!-- UPLOAD_LIST -->");
  if (placeholder_pos != std::string::npos) {
      html_content.replace(placeholder_pos, std::string("<!-- UPLOAD_LIST -->").length(), file_list_);
  } else {
    html_content += "<!-- Error: Upload list placeholder not found -->";
  }
  return html_content;
}

const std::map<std::string, std::string>& HttpParser::getContTypeMap() {
  static const std::map<std::string, std::string> cont_type_map = {
    {"audio/mpeg", ".mp3"},
    {"audio/x-ms-wma", ".wma"},
    {"audio/x-wav", ".wav"},

    {"image/png", ".png"},
    {"image/jpeg", ".jpg"},
    {"image/gif", ".gif"},
    {"image/tiff", ".tiff"},
    {"image/x-icon", ".ico"},
    {"image/vnd.djvu", ".djvu"},
    {"image/svg+xml", ".svg"},

    {"text/css", ".css"},
    {"text/csv", ".csv"},
    {"text/html", ".html"},
    {"text/plain", ".txt"},

    {"video/mp4", ".mp4"},
    {"video/x-msvideo", ".avi"},
    {"video/x-ms-wmv", ".wmv"},
    {"video/x-flv", ".flv"},
    {"video/webm", ".webm"},

    {"application/pdf", ".pdf"},
    {"application/json", ".json"},
    {"application/xml", ".xml"},
    {"application/zip", ".zip"},
    {"application/javascript", ".js"},
    {"application/vnd.oasis.opendocument.text", ".odt"},
    {"application/vnd.oasis.opendocument.spreadsheet", ".ods"},
    {"application/vnd.oasis.opendocument.presentation", ".odp"},
    {"application/vnd.oasis.opendocument.graphics", ".odg"},
    {"application/vnd.ms-excel", ".xls"},
    {"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", ".xlsx"},
    {"application/vnd.ms-powerpoint", ".ppt"},
    {"application/vnd.openxmlformats-officedocument.presentationml.presentation", ".pptx"},
    {"application/msword", ".doc"},
    {"application/vnd.openxmlformats-officedocument.wordprocessingml.document", ".docx"},
    {"application/vnd.mozilla.xul+xml", ".xul"}
};
return cont_type_map;
}