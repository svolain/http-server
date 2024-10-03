/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vsavolai <vsavolai@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 13:13:54 by vsavolai          #+#    #+#             */
/*   Updated: 2024/10/02 14:45:08 by vsavolai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"

extern bool show_request;
extern bool show_response;

HttpParser::HttpParser(const std::string request): error_code_(0), is_chunked_(false) {
  ParseRequest(request);
}
      
bool HttpParser::ParseRequest(const std::string request) {
    std::istringstream requestStream(request);
    std::string line;
    
    if (!std::getline(requestStream, line)) {
        std::cerr << "Error: getline failed to read request" << std::endl;
        //respond with 500 Internal Server Error
        error_code_ = 500;
        return false;
    }
    
    std::istringstream linestream(line);
    linestream >> method_ >> resource_path_ >> http_version_;

    if (method_.empty() || resource_path_.empty() || http_version_.empty()) {
        std::cerr << "Error: bad request 400" << std::endl;
        //respond with HTTP 400 Bad Request
        error_code_ = 400;
        return false;
    }
    
    std::string allowedMethods[] = {"GET", "POST", "DELETE", "HEAD"};
    if (std::find(std::begin(allowedMethods), std::end(allowedMethods), method_) ==
    std::end(allowedMethods)) {
        std::cerr << "Error: not supported method requested" << std::endl;
        //respond with HTTP 405 Method Not Allowed Error
        error_code_ = 405;
        return false;
    }


    size_t queryPosition = resource_path_.find("?");
    if (queryPosition != std::string::npos) {
        query_string_ = resource_path_.substr(queryPosition + 1);
        resource_path_ = resource_path_.substr(0, queryPosition);       
    }

    //call a function that checks if path exists
        if (!CheckValidPath(resource_path_)) {
            return false;
        }

    while (std::getline(requestStream, line) && line != "\r") {
        size_t delim = line.find(":");
        if (delim == std::string::npos) {
            std::cerr << "Error: wrong header line format" << std::endl;
            //respond with http 400 Bad Request
            error_code_ = 400;
            return false;
        }
        std::string header = line.substr(0, delim);
        std::string headerValue = line.substr(delim + 1);
        headerValue.erase(0, headerValue.find_first_not_of(" \t"));
        headers_[header] = headerValue;
    }

    if (method_ == "POST" || method_ == "PUT") {

        if (headers_["transfer-encoding"] == "chunked")
            is_chunked_ = true;
        int contentLength = 0;
        if (headers_.find("Content-Length") == headers_.end() && is_chunked_ == false) {
            std::cerr << "Error: content-lenght missing for request body" << std::endl;
            //respond with http 411 Length Required or general http 400 Bad Request?
            error_code_ = 411;
            return false;
        } else if (headers_.find("Content-Length") != headers_.end() && is_chunked_ == false) {
            std::string contentLengthStr = headers_["Content-Length"];
            try {
                contentLength = std::stoi(contentLengthStr);
                //probably have to also add maximum size also and check it in thos same row
                if (contentLength < 0) {
                    throw std::invalid_argument("Negative content length");
                }
                // Continue reading body as before
            } catch (const std::invalid_argument& e) {
                std::cerr << "Error: invalid Content-Length" << std::endl;
                error_code_ = 400; // Bad Request for invalid Content-Length
                return false;
            } catch (const std::out_of_range& e) {
                std::cerr << "Error: Content-Length out of range" << std::endl;
                error_code_ = 400; // Bad Request for excessively large Content-Length
                return false;
            }
        }
        
        /* Beware of off by 1, should be good tho */
        if (is_chunked_ == true)
        {
            std::streampos current = requestStream.tellg();
            requestStream.seekg(0, std::ios::end);
            contentLength = requestStream.tellg() - current;
            requestStream.seekg(current);
            chunk_size_ = contentLength;
        }
        requestStream.read(&request_body_[0], contentLength);
    }
    return true;

}

size_t HttpParser::get_chunk_size() const {
    return chunk_size_;
}
        
std::string HttpParser::get_method() const {
    return method_;
}

std::string HttpParser::get_resource_path() const {
    return resource_path_;
}

std::string HttpParser::get_query_string() const {
    return query_string_;
}

std::string HttpParser::get_http_version() const{
    return http_version_;
}

std::array<char, MAXBYTES>& HttpParser::get_request_body() {
    return request_body_;
}

std::map<std::string, std::string>& HttpParser::get_headers() {
    return headers_;
}

int HttpParser::get_error_code() const {
    return error_code_;   
}

bool HttpParser::get_is_chunked() const {
    return is_chunked_;
}

bool HttpParser::CheckValidPath(std::string path) {

    error_code_ = 200;
    /*for this function the root from confiq file is needed
    in short this searches the asked path either directory or file
    within the root directory*/
    if (path.at(0) != '/') {
        std::cerr << "Error: wrong path" << std::endl;
        error_code_ = 404; // or 400?
        return false;
    }

    //untill we get root it has to be set manually for example "www" in current working directory
    std::string rootPath = "";
     try {
        rootPath = std::filesystem::current_path().string() + "/www" + path;
        if (show_request)
        std::cout << "root path: " << rootPath << std::endl;
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        error_code_ = 500;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        error_code_ = 500;
        return false;
    }

    /*check the directory and file existence and permissions, 
    can take absolute and relative path, needs to be tested 
    more when we get the root from confiq*/
    try {
        if (rootPath.back() == '/') {
            if (std::filesystem::exists(rootPath) && std::filesystem::is_directory(rootPath)){
                std::cout << "valid path" << std::endl;
                return true;
            } else {
                error_code_ = 404;
                std::cout << "no valid path" << std::endl;
                return false;
            }
        } else {
            if (std::filesystem::exists(rootPath) && std::filesystem::is_regular_file(rootPath)){
                if (access(rootPath.c_str(), R_OK) == 0) {
                    if (show_request)
                      std::cout << "file found" << std::endl;
                    return true;
                } else {
                    std::cout << "permission denied" << std::endl;
                    error_code_ = 403;
                    return false;
                } 
            } else {
                std::cout << "file not found" << std::endl;
                error_code_ = 404;
                return false;
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
        error_code_ = 500;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        error_code_ = 500;
        return false;
    }

    return true;
}