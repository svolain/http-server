/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualHost.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 17:35:52 by  dshatilo         #+#    #+#             */
/*   Updated: 2024/10/03 13:05:54 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VirtualHost.hpp"
#include "ConnectInfo.hpp"

extern bool show_request;
extern bool show_response;

#define MAXBYTES  8192

VirtualHost::VirtualHost(std::string& max_size,
                         StringMap& errors,
                         LocationMap& locations)
    : locations_(locations) {
  if (!max_size.empty()) {
    client_max_body_size_ = std::stoi(max_size);
    client_max_body_size_ *= (max_size.back() == 'M' ? 1048576 : 1024);
  }
  for (const auto& [key, value] : errors)
    error_pages_[key] = value;
}

int VirtualHost::ParseHeader(ConnectInfo* fd_info, pollfd& poll) {

  char                buf[MAXBYTES]{};
  int                 bytesIn;
  size_t              request_size = 0;

  int fd = fd_info->get_fd();
  bytesIn = recv(fd, buf, MAXBYTES, 0);
  if (bytesIn < 0)
    return 1;
  else if (bytesIn == 0) {
    /* When a stream socket peer has performed an orderly shutdown, the
      return value will be 0 (the traditional "end-of-file" return) */
    return 2;
  }
  else if (bytesIn == MAXBYTES)
    std::cout << "MAXBYTES on recv. Check if the header is too long" << std::endl;

  request_size += bytesIn;
  
  if (show_request)
    std::cout << "\nthe whole request is:\n" << buf << std::endl;
    
  if (!fd_info->get_parser()->ParseRequest(buf))
    std::cout << "false on ParseRequest returned" << std::endl;
  if (request_size > SIZE_MAX)
    /* TODO: add body too long check in the parser */ ;
  if (fd_info->get_vhost() == nullptr)
    fd_info->AssignVHost();
  
  /* If the message didnt fit into MAXBYTES then dont set the POLLOUT yet,
    let the WriteBody do that */
  poll.events = POLLOUT;
  /* Set to true if we want to read the body 
    If the whole message fit into MAXBYTES then dont set it to true*/
  fd_info->set_is_parsing_body(false);
  
  return 0;
}

int VirtualHost::WriteBody(ConnectInfo* fd_info, pollfd& poll) {

  std::string&        request_body = fd_info->get_parser()->get_request_body();
  size_t              body_size = request_body.size();
  // std::vector<char>   buf(body_size + MAXBYTES);
  // buf.insert(buf.end(), request_body.begin(), request_body.end());

  int                 bytesIn;
  size_t              request_size = 0;

  int fd = fd_info->get_fd();
  bytesIn = recv(fd, &request_size + body_size, MAXBYTES, 0);
  if (bytesIn < 0)
    return 1;
  else if (bytesIn == 0) {
    /* When a stream socket peer has performed an orderly shutdown, the
      return value will be 0 (the traditional "end-of-file" return) */
    return 2;
  }
  else if (bytesIn == MAXBYTES)
    std::cout << "the header is too long! handle this" << std::endl;

  request_size += bytesIn;
  
  poll.events = POLLOUT;
  return 0;
}

void VirtualHost::OnMessageRecieved(ConnectInfo *fd_info, pollfd &poll){

  if (show_response)
    std::cout << "--- entering OnMessageRecieved ---" << std::endl;

  if (fd_info->get_is_sending() == false){
      SendHeader(fd_info);
      fd_info->set_is_sending(true);
  }
  else
    SendChunkedBody(fd_info, poll);
    
  if (show_response)
    std::cout << "----- leaving OnMessageRecieved -----" << std::endl;
}

void VirtualHost::SendHeader(ConnectInfo *fd_info){
  HttpParser *parser = fd_info->get_parser();
  std::string resource_path = parser->get_resource_path();

  if (show_request)
    std::cout << "the resource path is " << resource_path << std::endl;
  if (show_response)
    std::cout << "the error code from parser is " << parser->get_error_code() << std::endl;
    
  HttpResponse response;
  response.set_error_code_(parser->get_error_code());
  response.AssignContType(parser->get_resource_path());
  std::ifstream& file = fd_info->get_file();
  response.OpenFile(resource_path, file);
  response.ComposeHeader();

  if (show_response){
    std::cout << "\n------response header------" << std::endl;
    std::cout << response.get_header_() << std::endl;
    std::cout << "-----end of response header------\n" << std::endl;
  }
  
  SendToClient(fd_info->get_fd(), response.get_header_().c_str(), response.get_header_().size());
}

void VirtualHost::SendChunkedBody(ConnectInfo* fd_info, pollfd &poll)
{
  /* Nginx will not try to save the state of the previous transmission and retry later. 
      It handles each request-response transaction independently. 
      If the connection breaks, a client would need to send a new request to get the content again. 
      Return the file position back to 0*/

  HttpParser *parser = fd_info->get_parser();    
  std::string resource_path = parser->get_resource_path();
  std::ifstream& file = fd_info->get_file();
  HttpResponse response;
  response.OpenFile(resource_path, file);
  int client_socket = fd_info->get_fd();
  if (file.is_open()){
    if (SendOneChunk(client_socket, file) == 0){
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
  fd_info->set_is_sending(false);
  
  if (show_response)
    std::cout << "\n-----response sent-----\n" << std::endl;
}

int VirtualHost::SendOneChunk(int client_socket, std::ifstream &file)
{
  std::streamsize bytes_read;
  const int chunk_size = 1024;
  char buffer[chunk_size]{};

  file.read(buffer, chunk_size);
  bytes_read = file.gcount(); 
  if (bytes_read == -1){
    std::cout << "bytes_read returned -1" << std::endl;
    return 1;
  }
  std::ostringstream chunk_size_hex;
  chunk_size_hex << std::hex << bytes_read << "\r\n";
  if (SendToClient(client_socket, chunk_size_hex.str().c_str(), 
    chunk_size_hex.str().length()) == -1 || 
    SendToClient(client_socket, buffer, bytes_read) == -1 ||
    SendToClient(client_socket, "\r\n", 2) == -1){
      perror("send :");
      return 1;
  }
  if (show_response)
    std::cout << "sent " << bytes_read  << std::endl;
  if (bytes_read < chunk_size)
    return 1;
  return 0;
}

int VirtualHost::SendToClient(const int client_socket, const char *msg, int length){
  int bytes_sent;
  /* https://stackoverflow.com/questions/108183/how-to-prevent-sigpipes-or-handle-them-properly */
  bytes_sent = send(client_socket, msg, length, MSG_NOSIGNAL);
  if (bytes_sent != length)
    return -1;
  return bytes_sent;
}

size_t VirtualHost::get_max_body_size(){
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
