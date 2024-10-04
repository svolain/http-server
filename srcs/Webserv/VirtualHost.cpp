/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualHost.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 17:35:52 by  dshatilo         #+#    #+#             */
/*   Updated: 2024/10/04 16:33:56 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <sys/stat.h>
#include "VirtualHost.hpp"
#include "ClientInfo.hpp"
#include "Logger.h"
#include <string>

#define MAXBYTES  8192

VirtualHost::VirtualHost(std::string& max_size,
                         StringMap& errors,
                         LocationMap& locations)
    : locations_(locations) {
  if (!max_size.empty()) {
    client_max_body_size_ = std::stoi(max_size);
    client_max_body_size_ *= (max_size.back() == 'M' ? 1048576 : 1024);
  }
  for (const auto& [key, value] : errors) {
    if (access(value.c_str(), R_OK) == -1)
      continue;
    error_pages_[key] = value;
  }
}

int VirtualHost::ParseHeader(ClientInfo& fd_info, pollfd& poll) {

  char                buf[MAXBYTES]{};
  int                 bytesIn;
  HttpParser&         parser = fd_info.getParser();

  int fd = fd_info.getFd();
  bytesIn = recv(fd, buf, MAXBYTES, 0);
  if (bytesIn < 0)
    return 1;
  else if (bytesIn == 0) {
    /* When a stream socket peer has performed an orderly shutdown, the
      return value will be 0 (the traditional "end-of-file" return) */
    return 2;
  }
  else if (bytesIn == MAXBYTES)
    logDebug("MAXBYTES on recv. Check if the header is too long");
  
  logDebug("request is:" + std::string(buf));

  if (!parser.ParseRequest(buf))
    logDebug("false on ParseRequest returned", true);

  if (fd_info.getVhost() == nullptr)
    fd_info.AssignVHost();
  
  /* If the message didnt fit into MAXBYTES then dont set the POLLOUT yet,
    let the WriteBody do that */
  if (parser.getIsChunked() == true)
    fd_info.setIsParsingBody(true);
  else
    poll.events = POLLOUT;
  /* Set to true if we want to read the body 
    If the whole message fit into MAXBYTES then dont set it to true*/

  return 0;
}

int VirtualHost::WriteBody(ClientInfo& fd_info, pollfd& poll) {

  std::ofstream       &posftile = fd_info.getPostfile();
  HttpParser&         parser = fd_info.getParser();
  std::vector<char>&  request_body = parser.getRequestBody();
  size_t              body_size = request_body.size();
  
  //open the file
  if (!posftile.is_open() && !fd_info.getIsFileOpened()) {
    std::string filename = "./www/upload" + fd_info.getParser().getHeaders().at("POST") + 
      std::to_string(fd_info.getFd()) + "__temp__";
    posftile.open(filename, std::ios::binary);
    if (!posftile.is_open()) {
      parser.setErrorCode(500);
      poll.events = POLLOUT;
      return 1;
    }
    fd_info.setIsFileOpened(true);
  }
  else if (!posftile.is_open()) {
    //file close sometime during the write
    parser.setErrorCode(500);
    poll.events = POLLOUT;
    return 1;
  }
  //clear the chunk
  if (!UnChunkBody(request_body)) {
    parser.setErrorCode(400);
    if (fd_info.getPostfile().is_open()) {
      // fd_info.getPostfile().
      // std::remove (filename)
      // close the Postfile;
    }
    poll.events = POLLOUT;
    return 2;
  }

  posftile.write(request_body.data(), body_size);



  int                 bytesIn;
  size_t              request_size = 0;
  int fd = fd_info.getFd();
  bytesIn = recv(fd, request_body.data() + body_size, MAXBYTES, 0);
  if (bytesIn < 0)
    return 1;
  else if (bytesIn == 0) {
    /* When a stream socket peer has performed an orderly shutdown, the
      return value will be 0 (the traditional "end-of-file" return) */
    return 2;
  }
  else if (bytesIn == MAXBYTES)
    logDebug("bytesIn == MAXBYTES, more data to recieve");

  request_size += bytesIn;
  poll.events = POLLOUT;
  return 0;
}

bool VirtualHost::UnChunkBody(std::vector<char>& buf) {
  std::size_t readIndex = 0;
  std::size_t writeIndex = 0;

  while(readIndex < buf.size()) {
    std::size_t chunkSizeStart = readIndex;

    while (readIndex < buf.size() && !(buf[readIndex] == '\r' && buf[readIndex + 1] == '\n')) {
      readIndex++;
    }

    if (readIndex >= buf.size()) {
      logError("UnChunkBody: \\r\\n missing");
      return false;
    }

    std::string chunkSizeStr(buf.begin() + chunkSizeStart, buf.begin() + readIndex);
    std::size_t chunkSize;
    std::stringstream ss;
    ss << std::hex << chunkSizeStr;
    ss >> chunkSize;

    readIndex += 2;

    if (chunkSize == 0) {
            break;
        }

    if (readIndex + chunkSize > buf.size()) {
      logError("UnChunkBody: empty line missing");
      return false;
    }

    for (std::size_t i = 0; i < chunkSize; ++i) {
      buf[writeIndex++] = buf[readIndex++];
    }

    if (buf[readIndex] == '\r' && buf[readIndex + 1] == '\n') {
      readIndex += 2;
    } else {
      logError("UnChunkBody: \\r\\n missing");
      return false;
    }

  }
  buf.resize(writeIndex); 
  return true;
}


void VirtualHost::OnMessageRecieved(ClientInfo& fd_info, pollfd& poll) {

  logDebug("--- entering OnMessageRecieved ---", false);

  if (fd_info.getIsSending() == false) {
      SendHeader(fd_info);
      fd_info.setIsSending(true);
  }
  else
    SendChunkedBody(fd_info, poll);
    
  logDebug("----- leaving OnMessageRecieved -----", false);
}

void VirtualHost::SendHeader(ClientInfo& fd_info) {
  HttpParser& parser = fd_info.getParser();
  std::string resource_path = parser.getResourcePath();

  logDebug("the resource path is " + resource_path);
  logDebug("the error code from parser is " + std::to_string(parser.getErrorCode()));
  
  HttpResponse response;
  response.setErrorCode(parser.getErrorCode());
  response.AssignContType(parser.getResourcePath());
  std::ifstream& file = fd_info.getGetfile();
  response.OpenFile(resource_path, file);
  response.ComposeHeader();

  logDebug("\n------response header------\n" + \
            response.getHeader() + "\n" +\
            "-----end of response header------\n", false);
  
  SendToClient(fd_info.getFd(), response.getHeader().c_str(), response.getHeader().size());
}

void VirtualHost::SendChunkedBody(ClientInfo& fd_info, pollfd &poll)
{
  /* Nginx will not try to save the state of the previous transmission and retry later. 
      It handles each request-response transaction independently. 
      If the connection breaks, a client would need to send a new request to get the content again. 
      Return the file position back to 0*/

  HttpParser& parser = fd_info.getParser();
  std::string resource_path = parser.getResourcePath();
  std::ifstream& file = fd_info.getGetfile();
  HttpResponse response;
  response.OpenFile(resource_path, file);
  int client_socket = fd_info.getFd();
  if (file.is_open()) {
    if (SendOneChunk(client_socket, file) == 0) {
      return ;
    } 
    if (SendToClient(client_socket, "0\r\n\r\n", 5) == -1)
      perror("send 2:");
  }
  else
    if (SendToClient(client_socket, "<h1>404 Not Found</h1>", 23) == -1)
      perror("send 3:");
  file.close();
  poll.events = POLLIN;
  fd_info.setIsSending(false);
  
  logDebug("\n-----response sent-----\n", false);
}

int VirtualHost::SendOneChunk(int client_socket, std::ifstream &file)
{
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

int VirtualHost::SendToClient(const int client_socket, const char *msg, int length) {
  int bytes_sent;
  /* https://stackoverflow.com/questions/108183/how-to-prevent-sigpipes-or-handle-them-properly */
  bytes_sent = send(client_socket, msg, length, MSG_NOSIGNAL);
  if (bytes_sent != length)
    return -1;
  return bytes_sent;
}

size_t VirtualHost::getMaxBodySize() {
  return client_max_body_size_;
}

std::string VirtualHost::ToString() const {
  std::string out;
  out += std::string(21, ' ') + "Error Pages:\n";
  for (const auto& [code, path] : error_pages_)
    out += std::string(34, ' ') + "Error " + code + ": " + path + "\n";
  out += std::string(21, ' ') + "Client_max_body_size: ";
  out += std::to_string(client_max_body_size_) + " bytes\n";
  for (const auto& [path, location] : locations_) {
    out += std::string(21, ' ') + "Location: " + path + "\n";
    out += location.ToString() + "\n";
  }
  return out;
}
