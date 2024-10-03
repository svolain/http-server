/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  dshatilo < dshatilo@student.hive.fi >     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 15:44:32 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/03 23:53:29 by  dshatilo        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include "ClientInfo.hpp"
#include "Logger.h"
// #include <map>

HttpResponse::HttpResponse()
  :  error_code_(404), cont_type_map_{}, cont_type_("text/html"), error_code_message_{}{
  InitContMap();
}

HttpResponse::~HttpResponse(){
    ;
}

void HttpResponse::OpenFile(std::string& resource_path, std::ifstream& file){
  
  if (!file.is_open())
  {
    logDebug("the file wasnt opened previously");
    if (resource_path != "/" && resource_path.find(".html") == std::string::npos) //read bin file
      file.open("./www/" + resource_path, std::ios::binary);
    else{
      if (resource_path == "/") {
        resource_path = "index.html";
       logDebug("return index html for the '/'");
      }
      file.open("./www/" + resource_path);
    }
    if (!file.is_open()){
      logDebug("couldnt open file " + resource_path + ", opening 404", true);
      setErrorCode(404);
      file.open("./www/404.html");
      return ;
    }
  }
}

void HttpResponse::AssignContType(std::string resource_path){
  try{
    auto it = cont_type_map_.find(resource_path.substr(resource_path.find_last_of('.')));
    if (it != cont_type_map_.end()){
      cont_type_ = it->second;
    }
  }
  catch (const std::out_of_range &e){
    logDebug("no '.' found, cont type out of range caught");
  }
}

void HttpResponse::lookupErrMessage(void){
  //TODO do some kind of a pair lookup for error codes with the parser?
  switch (error_code_)
  {
    case 200:
      error_code_message_ = "200 OK";
      break;
    
    case 400:
      error_code_message_ = "400 Bad Request";
      break;
    
    case 405:
      error_code_message_ = "405 Method Not Allowed Error";
      break;
    
    case 411:
      error_code_message_ = "411 Length Required";
      break;
    
    case 500:
      error_code_message_ = "500 Internal Server Error";
      break;
    
    default:
      error_code_message_ = "404 Not Found";
      break;
  }
}

void HttpResponse::ComposeHeader(void){
  lookupErrMessage();
  
  std::ostringstream oss;
	oss << "HTTP/1.1 " << this->getErrorCodeMessage() << "\r\n";
	oss << "Content-Type: " << cont_type_ << "\r\n";
  /* Content length should be used when sends not in chunks */
	// oss << "Content-Length: " << (*content).size()  << "\r\n";
  oss << "Transfer-Encoding: chunked" << "\r\n";
	oss << "\r\n";
	this->header_ = oss.str();

}


void HttpResponse::InitContMap(void){
  this->cont_type_map_ = {
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


void  HttpResponse::setErrorCode(int error_code_){
  this->error_code_ = error_code_;
}


std::string HttpResponse::getContType(void) const{
  return (cont_type_);
}


std::string HttpResponse::getHeader(void) const{
  return (header_);
}

std::string HttpResponse::getErrorCodeMessage() const
{
  return (error_code_message_);
}