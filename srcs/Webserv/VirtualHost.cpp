/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualHost.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 17:35:52 by  dshatilo         #+#    #+#             */
/*   Updated: 2024/09/24 14:05:41 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VirtualHost.hpp"


extern bool showResponse;
extern bool showRequest;                                 ;

void VirtualHost::on_message_recieved(const int client_socket, HttpParser &parser, pollfd &sock){
  
  std::cout << "--- entering on_message_recieved ---" << std::endl;
  //TODO: check if the header contains "Connection: close"
  
  if (showRequest)
    std::cout << "the resource path is " << parser.get_resource_path() << std::endl;
  
  HttpResponse response;
  
  response.set_error_code_(parser.get_error_code());
  response.assign_cont_type_(parser.get_resource_path()); 
  std::ifstream file;
  response.open_file(parser.get_resource_path(), file, files_pos);
  response.compose_header();
  if (showResponse){
    std::cout << "\n------response header------" << std::endl;
    std::cout << response.get_header_() << std::endl;
    std::cout << "-----end of response header------\n" << std::endl;
  }
  
  send_to_client(client_socket, response.get_header_().c_str(), response.get_header_().size());

  send_chunked_body(client_socket, file, parser.get_resource_path(), sock);
}

void VirtualHost::send_chunked_body(int client_socket, std::ifstream &file, std::string resourcePath, pollfd &sock)
{
  const int chunk_size = 1024;
  char buffer[chunk_size]{};
  /* TODO: implement sending on POLLOUT and test */
  /* Nginx will not try to save the state of the previous transmission and retry later. 
      It handles each request-response transaction independently. 
      If the connection breaks, a client would need to send a new request to get the content again. 
      Return the file position back to 0*/
  if (file.is_open()){
    std::streamsize bytes_read;
    
    while (file) {
      file.read(buffer, chunk_size);
      files_pos[resourcePath] = file.tellg();
      bytes_read = file.gcount(); 
      if (bytes_read == -1){
        std::cout << "bytes_read returned -1" << std::endl;
        break;
      }
      std::ostringstream chunk_size_hex;
      chunk_size_hex << std::hex << bytes_read << "\r\n";
      if (send_to_client(client_socket, chunk_size_hex.str().c_str(), 
        chunk_size_hex.str().length()) == -1 || 
        send_to_client(client_socket, buffer, bytes_read) == -1 ||
        send_to_client(client_socket, "\r\n", 2) == -1){
          perror("send :");
          break ;
      }
      std::cout << "sent " << bytes_read  << std::endl;
    }
    if (send_to_client(client_socket, "0\r\n\r\n", 5) == -1)
      perror("send 2:");
    files_pos[resourcePath] = 0;
  }
  else
    if (send_to_client(client_socket, "<h1>404 Not Found</h1>", 23) == -1)
      perror("send 3:");
  file.close();
  copyFDs_[i].
  std::cout << "\n-----response sent-----\n" << std::endl;
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