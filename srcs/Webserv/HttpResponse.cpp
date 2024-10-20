/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 15:44:32 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/19 18:12:53 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include "ClientInfo.hpp"
#include "Logger.h"

HttpResponse::HttpResponse(std::string& status)
  : header_("---"), status_(status), cont_type_("text/html"), status_message_{} {}

void HttpResponse::SendResponse(ClientInfo& fd_info, pollfd& poll) {  
  std::fstream&   file = fd_info.getGetfile();
  int             client_socket = fd_info.getFd();

  additional_headers_ = fd_info.getParser().getAddHeaders();

  if (!header_.empty() && !SendHeader(client_socket, fd_info.getParser().getRequestTarget()))
    return;
  if (file.is_open()) {
    if (SendOneChunk(client_socket, file) == 0)
      return ;
    if (SendToClient(client_socket, "0\r\n\r\n", 6) == -1)
      perror("send 2:");
  } else if (SendToClient(client_socket, "<h1>500 Internal Server Error</h1>\r\n0\r\n\r\n", 42) == -1) 
    perror("send 3:");
  
  file.close();
  poll.events = POLLIN;
  logDebug("\n-----response sent-----\n", true);
}

int HttpResponse::SendHeader(int client_socket, std::string request_target) {

  AssignContType(request_target);
  LookupStatusMessage();
  
  ComposeHeader();
  std::cout << header_ << std::endl;
  if (SendToClient(client_socket, header_.c_str(), header_.size()) != -1) {
    header_.clear();
    return 0;
  }
  else
    logError("SendResponse: error on send");
  return 1;
}

void HttpResponse::AssignContType(std::string request_target) {
  try{
    auto it = getContTypeMap().find(request_target.substr(request_target.find_last_of('.')));
    if (it != getContTypeMap().end())
      cont_type_ = it->second;
  }
  catch (const std::out_of_range &e) {
    logDebug("AssignContType: no '.' found in the filename");
  }
}

void HttpResponse::LookupStatusMessage(void) {
  auto it = getStatusMap().find(status_);
  if (it != getStatusMap().end()) {
    status_message_ = it->second;
  } else {
    logError("LookupStatusMessage: couldn't find the proper status message, assigning 500");
    logError("status was " + status_);
    status_message_ = "500 Internal Server Error";
  }
}

void HttpResponse::ComposeHeader(void) {
  std::ostringstream oss;
	oss << "HTTP/1.1 " << status_message_ << "\r\n";
	oss << "Content-Type: " << cont_type_ << "\r\n";
  oss <<  additional_headers_ << "\r\n";
  oss << "Transfer-Encoding: chunked" << "\r\n";
	oss << "\r\n";
	this->header_ = oss.str();
}

int HttpResponse::SendOneChunk(int client_socket, std::fstream& file) {
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
  if (SendToClient(client_socket, chunk_size_hex.str().c_str(), chunk_size_hex.str().length()) == -1 || 
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

void HttpResponse::ResetResponse() {
  header_ = "---";
  cont_type_ = "text/html";
  status_message_.clear();
  cont_type_.clear();
}

const std::map<std::string, std::string>& HttpResponse::getContTypeMap() {
  static const std::map<std::string, std::string> cont_type_map  = 
  {
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
  return cont_type_map;
}

const std::map<std::string, std::string>& HttpResponse::getStatusMap() {
  static const std::map<std::string, std::string> status_map  = 
  {
    {"200", "200 OK"},
    {"302", "302 Found"},
    {"400", "400 Bad Request"},
    {"404", "404 Not Found"},
    {"405", "405 Method Not Allowed"},
    {"411", "411 Length Required"},
    {"500", "500 Internal Server Error"}
  };
  return status_map;
}
