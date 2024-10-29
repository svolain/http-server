/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 13:13:54 by vsavolai          #+#    #+#             */
/*   Updated: 2024/10/29 16:31:44 by dshatilo         ###   ########.fr       */
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
  const auto& it = std::find_if(locations.begin(),
                                locations.end(),
                                [&](const LocationPair& pair) {
      return request_target_.find(pair.first) == 0;
  });

  if (it == locations.end()) {
    logError("Location not found");
    client_.status_ = "404";
    return false;
  }

  const Location& loc = it->second;
  if (loc.methods_.find(method_) == std::string::npos) {
    logError("Method not allowed");
      client_.status_ = "405";
      return false;
  }
  if (!loc.redirection_.first.empty()) {
    client_.status_ = loc.redirection_.first;
    additional_headers_ = "Location: " + loc.redirection_.second + "\r\n";
    additional_headers_ += "Content-Length: 0\r\n";
    // additional_headers_ += "Connection: close\r\n";
    return false;
  }

  if (!loc.redirection_.first.empty()) {
    client_.status_ = loc.redirection_.first;
    additional_headers_ = "Location: " + loc.redirection_.second + "\r\n";
    client_.stage_ = ClientConnection::Stage::kResponse;
    return true;
  }

  index_ = loc.index_;
  HandleCookies();
  request_target_ = loc.root_ + request_target_.substr(it->first.size());

  if (method_ == "GET" && !HandleGet(loc.autoindex_))
    return false;
  else if (method_ == "DELETE" && !HandleDeleteRequest())
    return false;
  // else if (method_ == "POST")
    // ;
  else if (method_ == "POST") {
    if (!IsBodySizeValid()) {
      logError("Body size too big");
      client_.status_ = "431";
      return false;
    }
    if (request_body_.empty()) {
      client_.stage_ = ClientConnection::Stage::kBody;
    } else if (!HandlePostRequest(request_body_)) {
        return false;
    }
     }
  return true;
}

bool HttpParser::WriteBody(std::vector<char>& buffer, int bytesIn) {
  std::string eoc = "0\r\n\r\n";
  if (is_chunked_) {
    if (bytesIn > 5 || !std::equal(buffer.begin(), buffer.end(), eoc.begin())) {
      logDebug("bytesIn == MAXBYTES, more data to recieve");
      AppendBody(buffer, bytesIn);
      if (!is_chunked_ &&
          (request_body_.size() > content_length_)) {
        logError("Error: Request Header Fields Too Large");
        client_.status_ = "431";
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
    logError("Request Header Fields Too Large");
    client_.status_ = "431";
    return false;
  }
  return true;
}

void HttpParser::ResetParser() {
  content_length_ = 0;
  method_.clear();
  request_target_.clear();
  query_string_.clear();
  request_body_.clear();
  headers_.clear();
  index_.clear();
  is_chunked_ = false;
}

std::string HttpParser::getHost() {
  return headers_["Host"];
}

std::string HttpParser::getMethod() const {
  return method_;
}

std::string HttpParser::getRequestTarget() const {
  return request_target_;
}

std::string HttpParser::getFileList() const {
  return file_list_;
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

  static std::array<std::string, 4> allowed_methods = {
      "GET", "POST", "DELETE", "HEAD"};
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
  request_target_.erase(request_target_.begin());

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
  while (std::getline(request_stream, line) && line != "\r") {
    size_t delim = line.find(":");
    if (delim == std::string::npos || line.back() != '\r') {
      logError("Wrong header line format");
      client_.status_ = "400";
      return false; 
    }
    line.pop_back();
    std::string header = line.substr(0, delim);
    std::string header_value = line.substr(delim + 1);
    headers_[header] = header_value;
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

  if (line != "\r") {
    logError("Request header fields too large");
    client_.status_ = "431";
    return false;
  }
  return true;
}

bool HttpParser::CheckPostHeaders() {
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
    additional_headers_ += "Set-Cookie: session_id=" + session_id_ + "\r\n";
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

    if (chunkSize == 0) {
      break;
    }

    if (readIndex + chunkSize > buf.size()) {
      logError("UnChunkBody: empty line missing");
      client_.status_ = "400";
      return false;
    }

    for (std::size_t i = 0; i < chunkSize; ++i) {
      buf[writeIndex++] = buf[readIndex++];
    }

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

bool HttpParser::HandlePostRequest(std::vector<char> request_body) {
  auto it = headers_.find("Content-Type");

  if (it == headers_.end()) {
    client_.status_ = "400";
    return false;
  }

  std::string contentType = it->second;
  content_type_ = contentType;
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
  if (contentType.find("multipart/form-data") != std::string::npos) {
    logDebug("Handling multipart form data");
    if (!HandleMultipartFormData(request_body, contentType)) {
      client_.status_ = "400";// Internal Server Error
      return false;
    }
  } else {
    logError("Unsupported Content-Type");
    client_.status_ = "415";
    return false;
  }
  std::string clientFd = std::to_string(client_.fd_);
  std::string filename = "/tmp/webserv/upload_list"  + clientFd;
  std::fstream& outFile = client_.file_;
  if (OpenFile(filename))
    return false;
  std::remove(filename.c_str());
  std::string htmlStr = InjectFileListIntoHtml("www/index.html");
  outFile << htmlStr;
  client_.stage_ = ClientConnection::Stage::kResponse;
  outFile.seekg(0);
  return true;
}

bool HttpParser::HandleMultipartFormData(const std::vector<char> &body,
                                         const std::string &contentType) {
  size_t boundaryPosition = contentType.find("boundary=");
  if (boundaryPosition == std::string::npos)
    return false;

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
      std::ofstream outFile("www/uploads/" + filename, std::ios::binary);
      if (outFile.is_open()) {
        outFile.write(content.data(), content.size());
        outFile.close();
        logDebug("File ", filename, " saved successfully");
      } else {
        logError("Failed to save file ");
        
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
  query_string_ = UrlDecode(query_string_);
  size_t delim = query_string_.find("=");
  if (delim == std::string::npos) {
    logError("Wrong query string format");
    client_.status_ = "400";
    return false; 
  }
  std::string queryname = query_string_.substr(delim + 1);

  std::string path = "./www/uploads/" + queryname;
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
  std::string htmlStr = InjectFileListIntoHtml("www/index.html");
  outFile << htmlStr;
  client_.stage_ = ClientConnection::Stage::kResponse;
  outFile.seekg(0);
  return true;
}

void HttpParser::GenerateFileListHtml() {
    file_list_ = "<ul>";
    try {
        for (const auto &entry : std::filesystem::directory_iterator("www/uploads")) {
            std::string filename = entry.path().filename().string();
            file_list_ += "<li>";
            file_list_ += "<span>" + filename + "</span>";
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

void HttpParser::CreateDirListing(std::string& directory) {
  std::string clientFd = std::to_string(client_.fd_);
  std::string filename = "/tmp/webserv/dir_list"  + clientFd;
  std::fstream& outFile = client_.file_;
  if (OpenFile(filename))
    return;
  std::remove(filename.c_str());

    
  outFile << "<html><body><h1>Index of " << directory << "</h1><ul>";
  try {
      for (const auto& entry : std::filesystem::directory_iterator(directory)) {
          std::string name = entry.path().filename().string();
          if (std::filesystem::is_directory(entry.path())) {
              outFile << "<li><a href=\"" << name << "/\">" << name << "/</a></li>";
          } else {
              outFile << "<li><a href=\"" << name << "\">" << name << "</a></li>";
          }
      }
  } catch (const std::filesystem::filesystem_error& e) {
      outFile << "<h1>Directory not found</h1>";
      logError("Error accessing directory: ", directory);
      client_.status_ = "500";
  }
  //request_target_ = "www/dir_list.html";
  outFile<< "</ul></body></html>";
  logDebug("Directory listing created");
  outFile.seekg(0); 
}


int HttpParser::OpenFile(std::string& filename) {
  logDebug(filename);
  std::fstream& file = client_.file_;
  file.open(filename, std::fstream::in | std::fstream::out| std::fstream::app | std::ios::binary);
  if (!file.is_open()) {
    logError("1");
    client_.status_ = "500";
    file.open(client_.vhost_->getErrorPage("500"));
    return 1;
  }
  return 0;
}


std::string HttpParser::getAdditionalHeaders() {
  return additional_headers_;
}

bool HttpParser::HandleGet(bool autoIndex) {
  if (autoIndex && index_.empty() && request_target_.back() == '/') {
    CreateDirListing(request_target_);
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