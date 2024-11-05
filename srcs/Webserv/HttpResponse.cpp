/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 15:44:32 by klukiano          #+#    #+#             */
/*   Updated: 2024/11/05 13:23:09 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include "ClientConnection.hpp"
#include "Logger.hpp"

HttpResponse::HttpResponse(ClientConnection& client)
    : client_(client),
      cont_type_("text/html") {}

int HttpResponse::SendResponse(pollfd& poll) {
  std::fstream&   file = client_.file_;
  int             client_socket = client_.fd_;
  int             send_status;

  if (header_.empty())
    return (SendHeader(client_socket, client_.parser_));
  if (file.is_open()) {
    send_status = SendOneChunk(client_socket, file);
    if (send_status == 0 || send_status == 1)
      return send_status;
  } else if (SendToClient(client_socket, "<h1>500 Internal Server Error</h1>\r\n", 37) == -1)
    return 1;
  file.close();
  poll.events = POLLIN;
  logDebug("\n-----response sent-----\n");
  return 0;
}

int HttpResponse::SendHeader(int client_socket, HttpParser& parser) {

  AssignContType(parser.getRequestTarget());
  LookupStatusMessage();
  ComposeHeader();
  if (SendToClient(client_socket, header_.c_str(), header_.size()) != -1) {
    return 0;
  } else {
    logError("SendHeader: error on send");
    return 1;
  }
}

void HttpResponse::AssignContType(std::string request_target) {
  try {
    auto it = getContTypeMap().find(request_target.substr(request_target.find_last_of('.')));
    if (it != getContTypeMap().end())
      cont_type_ = it->second;
    else
      cont_type_ = "text/html";
  } catch (const std::out_of_range &e) {
    logDebug("AssignContType: no '.' found in the filename");
  }
}

void HttpResponse::LookupStatusMessage(void) {
  auto it = getStatusMap().find(client_.status_);
  if (it != getStatusMap().end()) {
    status_message_ = it->second;
  } else {
    logError("LookupStatusMessage: couldn't find the proper status message for ", client_.status_, ", assuming CGI, assigning 200");
    status_message_ = "200 OK";
  }
}

void HttpResponse::ComposeHeader() {
  StringMap&         addhead = client_.additional_headers_;
  std::ostringstream oss;
  
	oss << "HTTP/1.1 " << status_message_ << "\r\n";
  client_.additional_headers_["Content-Type:"] = cont_type_;
  if (!addhead.empty()) {
    for (auto it : addhead)
      oss << it.first << " " << it.second << "\r\n";
  } 
  if (!addhead.empty() && addhead.find("Content-Length:") == addhead.end())
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
  std::ostringstream chunk_stream;
  chunk_stream << std::hex << bytes_read << "\r\n"; 
  // size_t chunk_hex_size = chunk_stream.str().length();
  chunk_stream.write(buffer, bytes_read);
  chunk_stream << "\r\n";
  if (bytes_read < chunk_size)
    chunk_stream << "0\r\n\r\n";
  if (SendToClient(client_socket, chunk_stream.str().c_str(), chunk_stream.str().length()) == -1)
    return 1;
  if (bytes_read < chunk_size)
    return 2;
  return 0;
}

int HttpResponse::SendToClient(const int client_socket, const char* msg, int length) {

  int bytes_sent;
  bytes_sent = send(client_socket, msg, length, MSG_NOSIGNAL);
  if (bytes_sent == -1) {
    logError("send returned -1");
    return 1;
  }
  else if (bytes_sent == 0 || bytes_sent != length) {
    logError("send returned 0 or less than length");
    return 1;
  }
  return 0;
}

void HttpResponse::ResetResponse() {
  header_.clear();
  cont_type_ = "text/html";
  status_message_.clear();
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
    {"300", "300 Multiple Choices"},
    {"301", "301 Moved Permanently"},
    {"302", "302 Found"},
    {"303", "303 See Other"},
    {"304", "304 Not Modified"},
    {"307", "307 Temporary Redirect"},
    {"308", "308 Permanent Redirect"},
    {"400", "400 Bad Request"},
    {"403", "403 Forbidden"},
    {"404", "404 Not Found"},
    {"405", "405 Method Not Allowed"},
    {"411", "411 Length Required"},
    {"413", "413 Content Too Large"},
    {"415", "415 Unsupported Media Type"},
    {"431", "431 Request Header Fields Too Large"},
    {"500", "500 Internal Server Error"},
    {"501", "501 Not Implemented"},
    {"502", "502 Bad Gateway"},
    {"504", "504 Gateway Timeout"},
    {"505", "505 HTTP Version Not Supported"},
  };
  return status_map;
}
