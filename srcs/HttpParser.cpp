/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vsavolai <vsavolai@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 13:13:54 by vsavolai          #+#    #+#             */
/*   Updated: 2024/09/11 10:50:15 by vsavolai         ###   ########.fr       */
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
        return false;
    }
    
    std::istringstream linestream(line);
    linestream >> method >> resourcePath >> httpVersion;   

    std::string allowedMethods[] = {"GET", "POST", "DELETE", "HEAD"};
    if (std::find(std::begin(allowedMethods), std::end(allowedMethods), method) ==
    std::end(allowedMethods)) {
        std::cerr << "Error: not supported method requested" << std::endl;
        //respond with HTTP 405 Method Not Allowed Error
        return false;
    }


    size_t queryPosition = resourcePath.find("?");
    if (queryPosition != std::string::npos) {
        queryString = resourcePath.substr(queryPosition + 1);
        resourcePath = resourcePath.substr(0, queryPosition);       
    }

    while (std::getline(requestStream, line) && line != "\r") {
        size_t delim = line.find(":");
        if (delim == std::string::npos) {
            std::cerr << "Error: wrong header line format" << std::endl;
            //respond with http 400 Bad Request
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