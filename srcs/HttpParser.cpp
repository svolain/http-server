/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vsavolai <vsavolai@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 13:13:54 by vsavolai          #+#    #+#             */
/*   Updated: 2024/09/11 13:56:47 by vsavolai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"

HttpParser::HttpParser(void) {}

HttpParser::HttpParser(const std::string request) {
    parseRequest(request);
}


HttpParser::~HttpParser(void) {
    
}
        
bool HttpParser::parseRequest(const std::string request) {
    std::istringstream requestStream(request);
    std::string line;
    
    if (!std::getline(requestStream, line)) {
        std::cerr << "Error: getline failed to read request" << std::endl;
        //respond with 500 Internal Server Error
        error_code = 500;
        return false;
    }
    
    std::istringstream linestream(line);
    linestream >> method >> resourcePath >> httpVersion;

    if (method.empty() || resourcePath.empty() || httpVersion.empty()) {
        std::cerr << "Error: bad request 400" << std::endl;
        //respond with HTTP 400 Bad Request
        error_code = 400;
        return false;
    }
    
    std::string allowedMethods[] = {"GET", "POST", "DELETE", "HEAD"};
    if (std::find(std::begin(allowedMethods), std::end(allowedMethods), method) ==
    std::end(allowedMethods)) {
        std::cerr << "Error: not supported method requested" << std::endl;
        //respond with HTTP 405 Method Not Allowed Error
        error_code = 405;
        return false;
    }


    size_t queryPosition = resourcePath.find("?");
    if (queryPosition != std::string::npos) {
        queryString = resourcePath.substr(queryPosition + 1);
        resourcePath = resourcePath.substr(0, queryPosition);       
    }

    //call a function that checks if path exists, if not response 404
    if (!checkValidPath(resourcePath)) {
        return false;
    }

    while (std::getline(requestStream, line) && line != "\r") {
        size_t delim = line.find(":");
        if (delim == std::string::npos) {
            std::cerr << "Error: wrong header line format" << std::endl;
            //respond with http 400 Bad Request
            error_code = 400;
            return false;
        }
        std::string header = line.substr(0, delim);
        std::string headerValue = line.substr(delim + 1);
        headerValue.erase(0, headerValue.find_first_not_of(" \t"));
        headers[header] = headerValue;
    }

    /*
    if (method == "GET") {
        //read through asked file to see if there is images or another documents that
        // client will ask with new get request -> if there is then the client socket has
        // has to be left open
    }*/

    if (method == "POST" || method == "PUT") {
        
        if (headers.find("Content-Length") == headers.end()) {
            std::cerr << "Error: content-lenght missing for request body" << std::endl;
            //respoond with http 411 Length Required or general http 400 Bad Request?
            error_code = 411;
            return false;
        }
        std::string contentLengthStr = headers["Content-Length"];
        int contentLength = std::stoi(contentLengthStr);
        std::string body;
        body.reserve(contentLength);
        
        while (std::getline(requestStream, line)) {
            body += line + "\n";
        }
        requestBody = body;
    }
    return true;

}
        
std::string HttpParser::getMethod() const {
    return method;
}

std::string HttpParser::getResourcePath() const {
    return resourcePath;
}

std::string HttpParser::getQueryString() const {
    return queryString;
}

std::string HttpParser::getHttpVersion() const{
    return httpVersion;
}

std::string HttpParser::getrequestBody() const {
    return requestBody;
}

std::map<std::string, std::string> HttpParser::getHeaders() const {
    return headers;
}

int HttpParser::getErrorCode() const {
    return error_code;   
}

bool HttpParser::checkValidPath(std::string path) {
    /*for this function the root from confiq file is needed
    in short this searches the asked path either diirectory or file
    within the root directory, if not found rsponse is 404*/ 
    if (path.at(0) != '/') {
        std::cerr << "Error: wrong path" << std::endl;
        error_code = 404; // or 400?
        return false;
    }

    return true;
}