/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 15:44:32 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/15 13:18:14 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include "ClientInfo.hpp"
#include "Logger.h"

HttpResponse::HttpResponse(std::string& status)
  :  status_(status), cont_type_map_{}, cont_type_("text/html"), status_message_{}{
  InitContMap();
}

void HttpResponse::CreateResponse(ClientInfo& fd_info, pollfd& poll) {
  if (fd_info.getIsSending() == false) {
      SendHeader(fd_info);
      fd_info.setIsSending(true);
  }
  else
    SendChunkedBody(fd_info, poll);
}

void HttpResponse::ResetResponse() {
  ;//                                                      definition here         
}

void HttpResponse::SendHeader(ClientInfo& fd_info) {
  HttpParser& parser = fd_info.getParser();
  std::string resource_target = parser.getResourceTarget();
  /* reassign the default cont type */
  cont_type_ = "text/html";

  logDebug("the resource path is " + resource_target);
  logDebug("the error code from parser is " + status_);
  // setErrorCode(parser.getErrorCode());
  AssignContType(resource_target);
  std::ifstream& file = fd_info.getGetfile();
  OpenFile(resource_target, file);
  ComposeHeader();

  logDebug("\n------response header------\n" + \
            header_ + "\n" +\
            "-----end of response header------\n", true);

  SendToClient(fd_info.getFd(), header_.c_str(), header_.size());
}

void HttpResponse::SendChunkedBody(ClientInfo& fd_info, pollfd& poll) {
  /* Nginx will not try to save the state of the previous transmission and retry later. 
      It handles each request-response transaction independently. 
      If the connection breaks, a client would need to send a new request to get the content again. 
      Return the file position back to 0*/

  HttpParser& parser = fd_info.getParser();
  std::string resource_target = parser.getResourceTarget();
  std::ifstream& file = fd_info.getGetfile();
  OpenFile(resource_target, file);
  int client_socket = fd_info.getFd();
  if (file.is_open()) {
    if (SendOneChunk(client_socket, file) == 0) {
      return ;
    }
    if (SendToClient(client_socket, "0\r\n\r\n", 5) == -1)
      perror("send 2:");
  } else if (SendToClient(client_socket, "<h1>404 Not Found</h1>", 23) == -1) {
    logError("SendChunkedBody: couldn't open the file (most likely 404.html)");
    perror("send 3:");
  }
      
  file.close();
  poll.events = POLLIN;
  fd_info.setIsSending(false);

  logDebug("\n-----response sent-----\n", true);
}

int HttpResponse::SendOneChunk(int client_socket, std::ifstream& file) {
  std::streamsize bytes_read;
  const int chunk_size = 1024;
  char buffer[chunk_size]{};

  file.read(buffer, chunk_size);
  bytes_read = file.gcount();
  if (bytes_read == -1) {
    logDebug("bytes_read returned -1");
    return 1;
  }
  std::ostringstream chunk_size_hex;
  chunk_size_hex << std::hex << bytes_read << "\r\n";
  if (SendToClient(client_socket, chunk_size_hex.str().c_str(), 
    chunk_size_hex.str().length()) == -1 || 
    SendToClient(client_socket, buffer, bytes_read) == -1 ||
    SendToClient(client_socket, "\r\n", 2) == -1) {
      perror("send :");
      return 1;
  }

  logDebug("sent " + std::to_string(bytes_read), false);
  if (bytes_read < chunk_size)
    return 1;
  return 0;
}

int HttpResponse::SendToClient(const int client_socket, const char* msg, int length) {
  int bytes_sent;
  /* https://stackoverflow.com/questions/108183/how-to-prevent-sigpipes-or-handle-them-properly */
  bytes_sent = send(client_socket, msg, length, MSG_NOSIGNAL);
  if (bytes_sent != length) {
    logError("send: amount sent != amount requested for send");
    return -1;
  }
  return bytes_sent;
}

void HttpResponse::OpenFile(std::string& resource_path, std::ifstream& file) {
  
  if (!file.is_open()) {
    logDebug("the file wasnt opened previously");
    if (resource_path != "/" && resource_path.find(".html") == std::string::npos) //read bin file
      file.open("./www/" + resource_path, std::ios::binary);
    else {
      if (resource_path == "/") {
        resource_path = "index.html";
        logDebug("return index html for the '/'");
      }
      file.open("./www/" + resource_path);
    }
    if (!file.is_open()) {
      logDebug("couldnt open file " + resource_path + ", opening 404", true);
      status_ = "404";
      file.open("./www/404.html");
      return ;
    }
  }
}

void HttpResponse::AssignContType(std::string resource_path) {
  try{
    auto it = cont_type_map_.find(resource_path.substr(resource_path.find_last_of('.')));
    if (it != cont_type_map_.end()) {
      cont_type_ = it->second;
    }
  }
  catch (const std::out_of_range &e) {
    logDebug("no '.' found, cont type out of range caught");
  }
}

void HttpResponse::LookupStatusMessage(void) {
    std::map<std::string, std::string> status_map = {
        {"200", "200 OK"},
        {"400", "400 Bad Request"},
        {"405", "405 Method Not Allowed"},
        {"411", "411 Length Required"},
        {"500", "500 Internal Server Error"},
    };

    auto it = status_map.find(status_);
    if (it != status_map.end()) {
        status_message_ = it->second;
    } else {
        logError("LookupStatusMessage: couldn't find the proper status message, assigning 404");
        status_message_ = "404 Not Found";
    }
}

void HttpResponse::ComposeHeader(void) {
  LookupStatusMessage();
  std::ostringstream oss;
	oss << "HTTP/1.1 " << status_message_ << "\r\n";
	oss << "Content-Type: " << cont_type_ << "\r\n";
  oss << "Transfer-Encoding: chunked" << "\r\n";
	oss << "\r\n";
	this->header_ = oss.str();
}

/* Put it to the higher level */
void HttpResponse::InitContMap(void) {
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
