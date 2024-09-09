/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vsavolai <vsavolai@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 13:13:54 by vsavolai          #+#    #+#             */
/*   Updated: 2024/09/09 17:18:24 by vsavolai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpParser.hpp"

HttpParser::HttpParser(void) {}

HttpParser::HttpParser(const std::string buffer) {
    parseRequest(buffer);
}


HttpParser::~HttpParser(void) {
    
}
        
bool HttpParser::parseRequest(const std::string buffer) {
    /*assumed request
    GET /index.html?name=test HTTP/1.1
    Host: localhost
    */

    

}
        
std::string HttpParser::getMethod() const {

}

std::string HttpParser::getResourcePath() const {

}

std::string HttpParser::getQueryString() const {

}

std::string HttpParser::getHttpVersion() const{

}

std::map<std::string, std::string> HttpParser::getHeaders() const {
    
}

bool parseRequestLine(const std::string &line) {

}

bool parseHeaders(const std::vector<std::string> &headerLines) {
    
}