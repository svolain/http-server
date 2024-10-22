/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 13:13:54 by vsavolai          #+#    #+#             */
/*   Updated: 2024/10/22 11:56:35 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"
#include "Logger.hpp"
#include "ClientConnection.hpp"



HttpParser::HttpParser(ClientConnection& client) : client_(client) {}

bool HttpParser::ParseHeader(const std::string& request) {
  std::istringstream  request_stream(request);

  if (!ParseStartLine(request_stream))
    return false;
  if (!ParseHeaderFields(request_stream))
    return false;
  if (method_ != "POST")
    return true;
  if (!CheckPostHeaders())
    return false;
  std::streampos current = request_stream.tellg();
  request_stream.seekg(0, std::ios::end); /* Beware of off by 1, should be good tho */
  size_t stream_size = request_stream.tellg() - current;
  request_stream.seekg(current);
  request_body_.reserve(stream_size);
  request_stream.read(request_body_.data(), stream_size);
  return true;
}

bool  HttpParser::HandleRequest() {
  if (!IsBodySizeValid()) {
    logError("Body size too big");
    client_.status_ = "431";
    return false;
  }

  const std::map<std::string, Location>& locations = client_.vhost_->getLocations();
  std::string                         location;
  bool                                auto_index;
  std::pair<std::string, std::string> redir;

  for (const auto& it : locations) {
    if (request_target_.find(it.first) == 0) {
      location = it.first;
      index_ = it.second.index_;
      auto_index = it.second.autoindex_;
      redir = it.second.redirection_;
    }
  } 
  
  if (location.empty() ||
      request_target_.substr(0, location.size()) != location) {
    logError("Location not found");
    client_.status_ = "404";
    return false;
  }

  std::string allowedMethods = locations.at(location).methods_;
  if (allowedMethods.find(method_) == std::string::npos) {
    logError("Method not allowed");
      client_.status_ = "405";
      return false;
  }

  if (!redir.first.empty()) {
    client_.status_ = redir.first;
    location_header_ = "Location: " + redir.second;
    return false;
  }

  std::string root_dir = locations.at(location).root_;
  std::string relative_path = "/" + request_target_.substr(location.size()); 
  std::string rootPath = root_dir.substr(1) + relative_path;

  if (method_ == "GET" && auto_index && index_.empty()) {
    CreateDirListing(rootPath);  
  } else {
    if (!CheckValidPath(rootPath))
      return false;
  }

  if (method_ == "GET" || method_ == "HEAD" || method_ == "DELETE")
    return false; //This function should return false only in case of an error.
    //Please add 4 functions for each method

  return true;
}


int HttpParser::WriteBody(std::vector<char>& buffer, int bytesIn) {
  std::string eoc = "0\r\n\r\n";
  if (is_chunked_) {
    if (bytesIn > 5 || !std::equal(buffer.begin(), buffer.end(), eoc.begin())) {
      logDebug("bytesIn == MAXBYTES, more data to recieve");
      AppendBody(buffer, bytesIn);
      /*if (!is_chunked_ &&
          (!IsBodySizeValid(vhost) || request_body_.size() > content_length_)) {
        logError("Error: Request Header Fields Too Large");
        client_.status_ = 431;
        return 1;
      }*/
      return 0;
    }
    UnChunkBody(request_body_);
  } else {
    AppendBody(buffer, bytesIn);
    if (bytesIn == MAXBYTES)
      return 0;
  }
  HandlePostRequest(request_body_);
  return 1;
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
  is_chunked_ = false;
}

std::string HttpParser::getHost() const {
  return headers_.at("Host");
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
    headers_["Host"] = "";
    return false;
  }
  if (line != "\r") {
    logError("Request Header Fields Too Large");
    client_.status_ = "431";
    return false;
  }
  return true;
}

bool HttpParser::CheckPostHeaders() {
  auto it = headers_.find("transfer-encoding");
  is_chunked_ = (it != headers_.end() && it->second == "chunked");
  if (!is_chunked_) {
    logError(is_chunked_);
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

void HttpParser::HandlePostRequest(std::vector<char> request_body) {
  auto it = headers_.find("Content-Type");

  if (it == headers_.end()) {
    client_.status_ = "400";
    return;
  }

  std::string contentType = it->second;

  if (contentType.find("application/x-www-form-urlencoded") !=
      std::string::npos) {
    logDebug("Handling URL-encoded form submission");
    if (!ParseUrlEncodedData(request_body)) {
      client_.status_ = "500";// Internal Server Error
      return;
    }
  } else if (contentType.find("multipart/form-data") != std::string::npos) {
    logDebug("Handling multipart form data");
    if (!HandleMultipartFormData(request_body, contentType)) {
      client_.status_ = "500";// Internal Server Error
      return;
    }
    GenerateFileListHtml();
    //std::cout << "fileist:\n" << file_list_;
  } else {
    logError("Unsupported Content-Type");
    client_.status_ = "415";
  }
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

bool HttpParser::ParseUrlEncodedData(const std::vector<char>& body) {
  std::string requestBody(body.begin(), body.end());
  std::string filename = "form_bin";

  std::istringstream stream(requestBody);
  std::string pair;

  std::ofstream outputFile(filename, std::ios::binary);
  if (!outputFile) {
    logError("Could not open file for writing.");
    return false;
  }

  while (std::getline(stream, pair, '&')) {
    size_t delim = pair.find('=');
    if (delim == std::string::npos) {
      logError("URL encoding has wrong format.");
      return false;
    }
    std::string key = pair.substr(0, delim);
    std::string value = pair.substr(delim + 1);

    size_t keyLength = key.size();
    outputFile.write(reinterpret_cast<const char*>(&keyLength),
                     sizeof(keyLength));
    outputFile.write(key.data(), keyLength);

    size_t valueLength = value.size();
    outputFile.write(reinterpret_cast<const char*>(&valueLength),
                     sizeof(valueLength));
    outputFile.write(value.data(), valueLength);
  }
  outputFile.close();
  return true;
}

bool HttpParser::IsPathSafe(const std::string& path) {
  if (path.find(".."))
    return false;

  //this function we can add more checks to check safety
  return true;
}

void HttpParser::HandleDeleteRequest() {
  std::string path = request_target_;
  logDebug("Handling DELETE request for path: ", path);

  if (!IsPathSafe(path)) {
    client_.status_ = "400";
    return;
  }

  if (std::ifstream(path)) {
    if (std::remove(path.c_str()) == 0) {
      logDebug("File deleted successfully");
      client_.status_ = "204";
    } else {
      logError("Failed to delete file");
      client_.status_ = "500";
    }
  } else {
    logError("File not found");
    client_.status_ = "404";
  }
}

void HttpParser::GenerateFileListHtml() {
  file_list_ += "<ul>";
  for (const auto &entry : std::filesystem::directory_iterator("www/uploads")) {
    std::string filename = entry.path().filename().string();
    file_list_ += "<li>";
    file_list_ += "<span>" + filename + "</span>";
    file_list_ += "<button onclick=\"deleteFile('filename.txt')\">""Delete</button>";
    file_list_ += "</li>";
  }
  file_list_ += "</ul>";
}

bool HttpParser::CheckValidPath(std::string rootPath) {
  if (request_target_.at(0) != '/') {
    logError("Wrong path");
    client_.status_ = "404";
    return false;
  }
  /*check the directory and file existence and permissions, 
  can take absolute and relative path, needs to be tested 
  more when we get the root from confiq*/
  try {
  if (std::filesystem::exists(rootPath)) {
    if (std::filesystem::is_directory(rootPath)) {
      if (rootPath.back() != '/')
        request_target_ = rootPath + "/" + index_;
      else
        request_target_ = rootPath + index_;
      return true; //The path is a directory
    } else if (std::filesystem::is_regular_file(rootPath)) {
      request_target_ = rootPath;
      return false; //The path is a file
    }
    } else {
      logError("The path does not exist", rootPath);
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

void HttpParser::CreateDirListing(std::string directory) {
  std::ofstream outFile("./www/dir_list.html");
    if (!outFile.is_open()) {
        logError("Could not open file: www/dir_list.html");
        client_.status_ = "500";
        request_target_ = "www/error_pages/500.html";
        return;
    }
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
    request_target_ = "www/dir_list.html";
    outFile<< "</ul></body></html>";
    outFile.close();
    logDebug("Directory listing written to ./www/dir_list.html");
}


void HttpParser::OpenFile() {

  if (client_.status_.front() == '3'){
    logDebug("redirections status code, not opening the file");
    return ;
  }
  
  std::fstream&  file = client_.file_;
  logDebug("the requeset_target_ is ", request_target_);
  logDebug("the error code is ", client_.status_);

  if (!access(request_target_.c_str(), F_OK) && !access(request_target_.c_str(), R_OK))
    file.open(request_target_, std::ios::in | std::ios::binary);
  else if (access(request_target_.c_str(), F_OK)) {
    file.open(client_.vhost_->getErrorPage("404"), std::ios::in | std::ios::binary);
    client_.status_ = "404";
  }
  else if (access(request_target_.c_str(), R_OK)) {
    file.open(client_.vhost_->getErrorPage("500"), std::ios::in | std::ios::binary);
    client_.status_ = "500";
  }
  
  if (!file.is_open()) {
    file.open(client_.vhost_->getErrorPage("500"), std::ios::in | std::ios::binary);
    client_.status_ = "500";
  }
}


std::string HttpParser::getLocationHeader() {
  return location_header_;
}