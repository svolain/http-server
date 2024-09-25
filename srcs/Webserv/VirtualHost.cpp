/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualHost.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 17:35:52 by  dshatilo         #+#    #+#             */
/*   Updated: 2024/09/25 17:11:26 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VirtualHost.hpp"
#include "ConnectInfo.hpp"

extern bool showResponse;
extern bool showRequest;

void VirtualHost::on_message_recieved(ConnectInfo *fd_info, std::vector<pollfd> &copyFDs){
  std::cout << "--- entering on_message_recieved ---" << std::endl;
  //TODO: check if the header contains "Connection: close"  
  std::ifstream file;
  if ((*fd_info).get_is_sending() == false){
      send_header(fd_info, file);
  }
  else{
    send_chunked_body(fd_info, file, copyFDs);
  }
}

void VirtualHost::send_header(ConnectInfo *fd_info, std::ifstream &file){
  HttpResponse response;
  HttpParser *parser = (*fd_info).get_parser();

  if (showRequest)
    std::cout << "the resource path is " << parser->get_resource_path() << std::endl;
  response.set_error_code_(parser->get_error_code());
  response.assign_cont_type_(parser->get_resource_path());
  if (parser->get_error_code() == 200)
    response.open_file(fd_info, file);
  response.compose_header();
   if (showResponse){
    std::cout << "\n------response header------" << std::endl;
    std::cout << response.get_header_() << std::endl;
    std::cout << "-----end of response header------\n" << std::endl;
  }
  send_to_client((*fd_info).get_fd(), response.get_header_().c_str(), response.get_header_().size());
}

void VirtualHost::send_chunked_body(ConnectInfo* fd_info, std::ifstream &file, std::vector<pollfd> &copyFDs)
{
  /* Nginx will not try to save the state of the previous transmission and retry later. 
      It handles each request-response transaction independently. 
      If the connection breaks, a client would need to send a new request to get the content again. 
      Return the file position back to 0*/
  int client_socket = (*fd_info).get_fd();
  auto files_pos = (*fd_info).get_file_map();
  std::string resource_path = (*fd_info).get_parser()->get_resource_path();

  if (file.is_open()){
    if (file){
      if (send_one_chunk(client_socket, file, resource_path, files_pos) == 0){
        file.close();
        return ;
      }
    } 
    if (send_to_client(client_socket, "0\r\n\r\n", 5) == -1)
      perror("send 2:");
    files_pos[resource_path] = 0;
  }
  else
    if (send_to_client(client_socket, "<h1>404 Not Found</h1>", 23) == -1)
      perror("send 3:");
  file.close();
  copyFDs[(*fd_info).get_pollFD_index()].events = POLLIN;
  (*fd_info).set_is_sending(false);
  std::cout << "\n-----response sent-----\n" << std::endl;
}

int VirtualHost::send_one_chunk(int client_socket, std::ifstream &file, std::string resourcePath, 
  auto files_pos_)
{
  std::streamsize bytes_read;
  const int chunk_size = 1024;
  char buffer[chunk_size]{};

  file.read(buffer, chunk_size);
  files_pos_[resourcePath] = file.tellg();
  bytes_read = file.gcount(); 
  if (bytes_read == -1){
    std::cout << "bytes_read returned -1" << std::endl;
    return 1;
  }
  std::ostringstream chunk_size_hex;
  chunk_size_hex << std::hex << bytes_read << "\r\n";
  if (send_to_client(client_socket, chunk_size_hex.str().c_str(), 
    chunk_size_hex.str().length()) == -1 || 
    send_to_client(client_socket, buffer, bytes_read) == -1 ||
    send_to_client(client_socket, "\r\n", 2) == -1){
      perror("send :");
      return 1;
  }
  std::cout << "sent " << bytes_read  << std::endl;
  if (bytes_read < chunk_size)
    return 1;
  return 0;
}

int VirtualHost::send_to_client(const int client_socket, const char *msg, int length){
  int bytes_sent;
  bytes_sent = send(client_socket, msg, length, 0);
  if (bytes_sent != length)
    return -1;
  return bytes_sent;
}

std::string VirtualHost::get_name() {
  return name_;
}

void VirtualHost::set_name(std::string& name) {
  name_ = name;
}

void VirtualHost::set_size(std::string& size) {
  client_max_body_size_ = std::stoi(size);
  client_max_body_size_ *= (size.back() == 'M' ? 1048576 : 1024);
}

void VirtualHost::set_error_page(std::string& code, std::string& path) {
  error_pages_[code] = path;
}

void VirtualHost::set_location(std::string& path, Location& location) {
  locations_[path] = location;
}

size_t VirtualHost::get_max_body_size()
{
  return client_max_body_size_;
}