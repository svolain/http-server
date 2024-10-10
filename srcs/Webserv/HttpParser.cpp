/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vsavolai <vsavolai@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 13:13:54 by vsavolai          #+#    #+#             */
/*   Updated: 2024/10/10 13:23:31 by vsavolai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"
#include "Logger.h"

HttpParser::HttpParser(const std::string request) //We don't use it yet
    : error_code_(0), is_chunked_(false) {
  ParseHeader(request);
}

bool HttpParser::ParseHeader(const std::string& request) {
  std::istringstream  request_stream(request);
  std::string         line;

  request_stream >> method_ >> resource_path_ >> http_version_;
  std::getline(request_stream, line);
   //what if http_version is incorrect?
  if (method_.empty() || resource_path_.empty()
      || http_version_.empty() || line != "\r") {
    logError("Error: bad request 400");
    //respond with HTTP 400 Bad Request
    error_code_ = 400;
    return false;
  }

  static std::array<std::string, 4> allowed_methods = {
      "GET", "POST", "DELETE", "HEAD"};
  if (std::find(allowed_methods.begin(), allowed_methods.end(), method_) ==
      allowed_methods.end()) {
    logError("Error: not supported method requested");
    //respond with HTTP 501 Not Implemented Error
    error_code_ = 501;
    return false;
  }

  size_t query_position = resource_path_.find("?");
  if (query_position != std::string::npos) {
    query_string_ = resource_path_.substr(query_position + 1);
    resource_path_ = resource_path_.substr(0, query_position);
  }

                        // if (!CheckValidPath(resource_path_)) {      //Path directories'll be checked later
                        //   return false;
                        // }

  while (std::getline(request_stream, line) && line != "\r") {
    size_t delim = line.find(":");
    if (delim == std::string::npos || line.back() != '\r') {
      logError("Error: wrong header line format");
      //respond with http 400 Bad Request
      error_code_ = 400;
      return false; 
    }
    line.pop_back();
    std::string header = line.substr(0, delim);
    std::string header_value = line.substr(delim + 1);
    header_value.erase(0, header_value.find_first_not_of(" \t"));
    headers_[header] = header_value;
    //Ideally for specific headers server should check for duplicates and send 400 if it finds them (two hosts or content-length for ex)
  }
  if (!headers_.contains("Host")) {
    logError("Error: bad request 400");
    error_code_ = 400;
    return false;
  }
  if (line != "\r") {
    logError("Error: Request Header Fields Too Large");
    error_code_ = 431;
    return false;
  }

  if (method_ == "GET" || method_ == "HEAD")
    return true;

  auto it = headers_.find("transfer-encoding");
  is_chunked_ = (it != headers_.end() && it->second == "chunked");

  if (!is_chunked_) {
    auto it = headers_.find("Content-Length");
    if (it == headers_.end()) {
      logError("Error: content-lenght missing for request body");
      //respond with http 411 Length Required
      error_code_ = 411;
      return false;
    } else {
      std::string& content_length_str = it->second;
      try {
        size_t pos;
        content_length_ = std::stoi(content_length_str, &pos);
        if (pos != content_length_str.size() || content_length_ < 0)
          throw std::invalid_argument("Invalid argument");
      } catch (const std::invalid_argument& e) {
        logError("Error: invalid Content-Length");
        error_code_ = 400; // Bad Request for invalid Content-Length
        return false;
      } catch (const std::out_of_range& e) {
        logError("Error: Content-Length out of range");
        error_code_ = 413; // Bad Request for excessively large Content-Length
        return false;
      }
    }
  }
  std::streampos current = request_stream.tellg();
  request_stream.seekg(0, std::ios::end); /* Beware of off by 1, should be good tho */
  size_t stream_size = request_stream.tellg() - current;
  request_stream.seekg(current);
  request_body_.reserve(stream_size);
  request_stream.read(request_body_.data(), stream_size);
  return true;
}

int HttpParser::WriteBody(VirtualHost* vhost, std::vector<char>& buffer,
                          int bytesIn) {
  std::string eoc = "0\r\n\r\n";

  if (bytesIn > 5 || !std::equal(buffer.begin(), buffer.end(), eoc.begin())) {
    logDebug("bytesIn == MAXBYTES, more data to recieve");
    AppendBody(buffer, bytesIn);
    if (!IsBodySizeValid(vhost) || request_body_.size() > content_length_) {
      logError("Error: Request Header Fields Too Large");
      error_code_ = 431;
      return 1;
    }
    return 0;
  }
  UnChunkBody(request_body_);
  return 1;
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
      setErrorCode(400);
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
      setErrorCode(400);
      return false;
    }

    for (std::size_t i = 0; i < chunkSize; ++i) {
      buf[writeIndex++] = buf[readIndex++];
    }

    if (buf[readIndex] == '\r' && buf[readIndex + 1] == '\n') {
      readIndex += 2;
    } else {
      setErrorCode(400);
      logError("UnChunkBody: \\r\\n missing");
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

bool  HttpParser::IsBodySizeValid(VirtualHost* vhost) {
  if (request_body_.size() > vhost->getMaxBodySize()) {
    logError("Error: Request Header Fields Too Large");
    error_code_ = 431;
    return false;
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
        std::cerr << "Error: could not open file for writing." << std::endl;
        return false;
    }

    while (std::getline(stream, pair, '&')) {
        size_t delim = pair.find('=');
        if (delim == std::string::npos) {
            std::cerr << "Error: URL encoding has wrong format." << std::endl;
            return false;
        }
        std::string key = pair.substr(0, delim);
        std::string value = pair.substr(delim + 1);

        size_t keyLength = key.size();
        outputFile.write(reinterpret_cast<const char*>(&keyLength), sizeof(keyLength));
        outputFile.write(key.data(), keyLength);

        size_t valueLength = value.size();
        outputFile.write(reinterpret_cast<const char*>(&valueLength), sizeof(valueLength));
        outputFile.write(value.data(), valueLength);
    }

    outputFile.close();
    return true;
}

bool HttpParser::ParseMultiPartData(std::vector<char> &bodyPart) {
    std::vector<char> crlf = {'\r', '\n'};
    
    auto headerEndIt = std::search(bodyPart.begin(), bodyPart.end(), crlf.begin(), crlf.end());
    if (headerEndIt == bodyPart.end()) {
        std::cerr << "Error: invalid multi part format" << std::endl;
        return false;
    }

    std::vector<char> headers(bodyPart.begin(), headerEndIt);
    std::vector<char> content(headerEndIt + 2, bodyPart.end());

    std::string headersStr(headers.begin(), headers.end());

    if (headersStr.find("Content-Disposition") != std::string::npos) {
        size_t posFilename = headersStr.find("filename=");
        size_t posName = headersStr.find("name=");

        if (posFilename != std::string::npos) {

            size_t posStart = headersStr.find('"', posFilename) + 1;
            size_t posEnd = headersStr.find('"', posStart);
            std::string filename = headersStr.substr(posStart, posEnd - posStart);

            std::cout << "File upload: " << filename << std::endl;

            std::ofstream outFile("www/uploads/" + filename, std::ios::binary);
            if (outFile.is_open()) {
                outFile.write(content.data(), content.size());
                outFile.close();
                std::cout << "File " << filename << " saved successfully." << std::endl;
            } else {
                std::cerr << "Error: failed to save file " << filename << std::endl;
                return false;
            }
        } else if (posName != std::string::npos) {
      
            size_t posStart = headersStr.find('"', posName) + 1;
            size_t posEnd = headersStr.find('"', posStart);
            std::string fieldName = headersStr.substr(posStart, posEnd - posStart);

            std::string contentStr(content.begin(), content.end());
            std::cout << "Form field: " << fieldName << " = " << contentStr << std::endl;
        }
    }

    return true;
}

bool HttpParser::HandleMultipartFormData(const std::vector<char> &body, const std::string &contentType) {
    std::cout << "Multipart form data received: " << body.size() << " bytes" << std::endl;

    size_t boundaryPosition = contentType.find("boundary=");
    if (boundaryPosition == std::string::npos)
        return false;

    std::string boundary = "--" + contentType.substr(boundaryPosition + 9);
    std::cout << "boundary:\n" << boundary << "\n";

    std::vector<char> boundaryVec(boundary.begin(), boundary.end());
    size_t boundaryLength = boundaryVec.size();

    auto pos = body.begin();

    while (true) {
        auto boundaryStart = std::search(pos, body.end(), boundaryVec.begin(), boundaryVec.end());
        if (boundaryStart == body.end())
            break;

        pos = boundaryStart + boundaryLength;

        auto nextBoundaryStart = std::search(pos, body.end(), boundaryVec.begin(), boundaryVec.end());

        std::vector<char> bodyPart(pos, nextBoundaryStart);

        if (!ParseMultiPartData(bodyPart))
            return false;

        pos = nextBoundaryStart;
    }

    return true;
}

void  HttpParser::HandlePostRequest(std::vector<char> request_body, HttpParser &parser) {
    std::string contentType = headers_.at("Content-Type");
    parser.setErrorCode(200);

  if (contentType.find("application/x-www-form-urlencoded") != std::string::npos) {
        std::cout << "Handling URL-encoded form submission" << std::endl;
        if (!ParseUrlEncodedData(request_body)) {
             parser.setErrorCode(500); // Internal Server Error
            return;
        }
        
    } else if (contentType.find("multipart/form-data") != std::string::npos) {
        if (!HandleMultipartFormData(request_body, contentType)) {
            parser.setErrorCode(500); // Internal Server Error
            return;
        }
    } else {
        std::cerr << "Unsupported Content-Type: " << contentType << std::endl;
        parser.setErrorCode(415);
    }
}

std::string  HttpParser::getHost() const {
  return headers_.at("Host");
}

std::string HttpParser::getMethod() const {
  return method_;
}

std::string HttpParser::getResourcePath() const {
  return resource_path_;
}

int HttpParser::getErrorCode() const {
  return error_code_;
}

void  HttpParser::setErrorCode(int error) {
  error_code_ = error;
}

// bool HttpParser::CheckValidPath(std::string path) {

//     error_code_ = 200;
//     /*for this function the root from confiq file is needed
//     in short this searches the asked path either directory or file
//     within the root directory*/
//     if (path.at(0) != '/') {
//         logError("Error: wrong path");
//         error_code_ = 404; // or 400?
//         return false;
//     }

//     //untill we get root it has to be set manually for example "www" in current working directory
//     std::string rootPath = "";
//      try {
//         rootPath = std::filesystem::current_path().string() + "/www" + path;
//         logDebug("root path: " + rootPath);
//     } catch (const std::filesystem::filesystem_error& e) {
//         logError("Filesystem error: ");
//         std::cerr << e.what() << std::endl;
//         error_code_ = 500;
//         return false;
//     } catch (const std::exception& e) {
//         logError("Unexpected error: "); 
//         std::cerr << e.what() << std::endl;
//         error_code_ = 500;
//         return false;
//     }

//     /*check the directory and file existence and permissions, 
//     can take absolute and relative path, needs to be tested 
//     more when we get the root from confiq*/
//     try {
//         if (rootPath.back() == '/') {
//             if (std::filesystem::exists(rootPath) && std::filesystem::is_directory(rootPath)) {
//                 logDebug("valid path");
//                 return true;
//             } else {
//                 error_code_ = 404;
//                 logDebug("no valid path");
//                 return false;
//             }
//         } else {
//             if (std::filesystem::exists(rootPath) && std::filesystem::is_regular_file(rootPath)) {
//                 if (access(rootPath.c_str(), R_OK) == 0) {
//                     logDebug("file found");
//                     return true;
//                 } else {
//                     logDebug("permission denied");
//                     error_code_ = 403;
//                     return false;
//                 } 
//             } else {
//                 logDebug("file not found");
//                 error_code_ = 404;
//                 return false;
//             }
//         }
//     } catch (const std::filesystem::filesystem_error& e) {
//         logError("Filesystem error: ");
//         std::cerr << e.what() << std::endl;
//         error_code_ = 500;
//         return false;
//     } catch (const std::exception& e) {
//         logError("Unexpected error: "); 
//         std::cerr << e.what() << std::endl;
//         error_code_ = 500;
//         return false;
//     }

//     return true;
// }
