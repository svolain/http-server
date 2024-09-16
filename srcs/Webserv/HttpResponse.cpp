/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 15:44:32 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/16 18:15:21 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"

extern bool showResponse;

HttpResponse::HttpResponse()
  :  errorCode(404), contTypeMap{}, contType("text/html"), errorCodeMessage{}{
  initContMap();
}

HttpResponse::~HttpResponse(){
    ;
}

void HttpResponse::openFile(std::string resourcePath)
{
  
  //open in binary if not html
  //TODO: make a check for the file extension in the parser
  if (resourcePath != "/" && resourcePath.find(".html") == std::string::npos) //read bin file
      this->file.open("./www/" + resourcePath, std::ios::binary);
  else
  {
    if (resourcePath == "/")
      resourcePath = "index.html";
    file.open("./www/" + resourcePath);
  }
  if (!file.is_open())
  {
    std::cout << "couldnt open file " << resourcePath << ", opening 404" << std::endl;
    file.open("./www/404.html");
  }
    

  //TODO: change the error code when the methd is gonna be of the responese class
}

void HttpResponse::assignContType(std::string resourcePath){
  try{
    auto it = contTypeMap.find(resourcePath.substr(resourcePath.find_last_of('.')));
    if (it != contTypeMap.end()){
      contType = it->second;
    }
  }
  catch (const std::out_of_range &e){
    std::cout << "no '.' found, cont type out of range caught" << std::endl;;
  }
}

void HttpResponse::lookupErrMessage(void)
{
  //TODO do some kind of a table duple lookup for error codes with the parser?
  switch (errorCode)
  {
    case 200:
      errorCodeMessage = "200 OK";
      break;
    
    case 400:
      errorCodeMessage = "400 Bad Request";
      break;
    
    case 405:
      errorCodeMessage = "405 Method Not Allowed Error";
      break;
    
    case 411:
      errorCodeMessage = "411 Length Required";
      break;
    
    case 500:
      errorCodeMessage = "500 Internal Server Error";
      break;
    
    default:
      errorCodeMessage = "404 Not Found";
      break;
  }
}

void HttpResponse::composeHeader(void){
  lookupErrMessage();
  
  std::ostringstream oss;
	oss << "HTTP/1.1 " << this->getErrorCodeMessage() << "\r\n";
	oss << "Content-Type: " << contType << "\r\n";
  /* Content length should be used when sends not in chunks */
	// oss << "Content-Length: " << (*content).size()  << "\r\n";
  oss << "Transfer-Encoding: chunked" << "\r\n";
	oss << "\r\n";
	this->header = oss.str();
}


void HttpResponse::initContMap(void)
{
  this->contTypeMap = {
    {".mp3", "audio/mpeg"},
    {".wma", "audio/x-ms-wma"},
    {".wav", "audio/x-wav"},

    {".png", "image/png"},
    {".jpg", "image/jpeg"},
    {".gif", "image/gif"},
    {".tiff", "image/tiff"},
    {".ico", "image/x-icon"},
    {".djvu", "image/vnd.djvu"},
    {".svg", "image/svg+xml"},

    {".css", "text/css"},
    {".csv", "text/csv"},
    {".html", "text/html"},
    {".txt", "text/plain"},

    {".mp4", "video/mp4"},
    {".avi", "video/x-msvideo"},
    {".wmv", "video/x-ms-wmv"},
    {".flv", "video/x-flv"},
    {".webm", "video/webm"},

    {".pdf", "application/pdf"},
    {".json", "application/json"},
    {".xml", "application/xml"},
    {".zip", "application/zip"},
    {".js", "application/javascript"},
    {".odt", "application/vnd.oasis.opendocument.text"},
    {".ods", "application/vnd.oasis.opendocument.spreadsheet"},
    {".odp", "application/vnd.oasis.opendocument.presentation"},
    {".odg", "application/vnd.oasis.opendocument.graphics"},
    {".xls", "application/vnd.ms-excel"},
    {".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"},
    {".ppt", "application/vnd.ms-powerpoint"},
    {".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"},
    {".doc", "application/msword"},
    {".docx", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"},
    {".xul", "application/vnd.mozilla.xul+xml"}
  };
}


void  HttpResponse::setErrorCode(int errorCode){
  this->errorCode = errorCode;
}


std::string HttpResponse::getContType(void) const{
  return (contType);
}

std::ifstream& HttpResponse::getFile(void){
  return (this->file);
}

std::string HttpResponse::getHeader(void) const{
  return (header);
}

std::string HttpResponse::getErrorCodeMessage() const
{
  return (errorCodeMessage);
}