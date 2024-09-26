/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 15:44:32 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/26 13:58:51 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include "ConnectInfo.hpp"
#include <map>

HttpResponse::HttpResponse()
  :  error_code_(404), cont_type_map_{}, cont_type_("text/html"), error_code_message_{}{
  initContMap();
}

HttpResponse::~HttpResponse(){
    ;
}

void HttpResponse::open_file(ConnectInfo *fd_info, std::ifstream &file)
{
  //open in binary if not html
  //TODO: make a check for the file extension in the parser
  std::string resourcePath = (*fd_info).get_parser()->get_resource_path();
  auto files_pos = (*fd_info).get_file_map();

  auto filesmap_it = files_pos.find(resourcePath);
  std::streampos pos = 0;
  if (filesmap_it != files_pos.end())
  {
    pos = (*filesmap_it).second;
    std::cout << "found the file for access in the map" << std::endl;
  }
  else
    files_pos[resourcePath] = 0;

  if (resourcePath != "/" && resourcePath.find(".html") == std::string::npos) //read bin file
      file.open("./www/" + resourcePath, std::ios::binary);
  else
  {
    if (resourcePath == "/")
      resourcePath = "index.html";
    std::cout << "set index html as default" << std::endl;
    file.open("./www/" + resourcePath);
  }
  if (!file.is_open())
  {
    std::cout << "couldnt open file " << resourcePath << ", opening 404" << std::endl;
    set_error_code_(404);
    file.open("./www/404.html");
    return ;
  }
  std::cout << "the pos for file is " << pos << std::endl;
  file.seekg(pos);
  if (file.fail()) {
    std::cerr << "Error: Failed to set the file position." << std::endl;
  } 
  else if (pos != 0){
        std::cout << "Previous file position set successfully." << std::endl;
  }
}

void HttpResponse::assign_cont_type_(std::string resourcePath){
  try{
    auto it = cont_type_map_.find(resourcePath.substr(resourcePath.find_last_of('.')));
    if (it != cont_type_map_.end()){
      cont_type_ = it->second;
    }
  }
  catch (const std::out_of_range &e){
    std::cout << "no '.' found, cont type out of range caught" << std::endl;;
  }
}

void HttpResponse::lookupErrMessage(void)
{
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

void HttpResponse::compose_header(void){
  lookupErrMessage();
  
  std::ostringstream oss;
	oss << "HTTP/1.1 " << this->get_error_codeMessage() << "\r\n";
	oss << "Content-Type: " << cont_type_ << "\r\n";
  /* Content length should be used when sends not in chunks */
	// oss << "Content-Length: " << (*content).size()  << "\r\n";
  oss << "Transfer-Encoding: chunked" << "\r\n";
	oss << "\r\n";
	this->header_ = oss.str();

}


void HttpResponse::initContMap(void)
{
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


void  HttpResponse::set_error_code_(int error_code_){
  this->error_code_ = error_code_;
}


std::string HttpResponse::get_cont_type_(void) const{
  return (cont_type_);
}


std::string HttpResponse::get_header_(void) const{
  return (header_);
}

std::string HttpResponse::get_error_codeMessage() const
{
  return (error_code_message_);
}