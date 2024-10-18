/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 15:44:32 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/18 17:55:39 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpResponse.hpp"
#include "ClientInfo.hpp"
#include "Logger.h"

#include <sys/stat.h>

/* How to init the map only once? */
// HttpResponse::HttpResponse()
//   :  {}


void HttpResponse::SendChunkedBody(ClientInfo& fd_info, pollfd& poll) {
  
  std::ifstream& file = fd_info.getGetfile();
  int client_socket = fd_info.getFd();
  if (file.is_open()) {
    if (SendOneChunk(client_socket, file) == 0)
      return ;
    if (SendToClient(client_socket, "0\r\n\r\n", 6) == -1)
      perror("send 2:");
  } 
  else if (SendToClient(client_socket, "<h1>500 Internal Server Error</h1>\r\n0\r\n\r\n", 42) == -1) 
    perror("send 3:");
  
  file.close();
  poll.events = POLLIN;
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
  cont_type_ = "text/html";
  status_message_.clear();
  header_.clear();
}