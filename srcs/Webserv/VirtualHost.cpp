/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualHost.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 17:35:52 by  dshatilo         #+#    #+#             */
/*   Updated: 2024/09/21 16:54:00 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VirtualHost.hpp"


extern bool showResponse;
extern bool showRequest;


void VirtualHost::on_message_recieved(const int clientSocket, HttpParser &parser){
  
  std::cout << "--- entering on_message_recieved ---" << std::endl;
  /* TODO if bytesIn == MAXBYTES then recv until the whole message is sent 
    see 100 Continue status message
    https://www.w3.org/Protocols/rfc2616/rfc2616-sec8.html#:~:text=Requirements%20for%20HTTP/1.1%20origin%20servers%3A*/

  //TODO: check if the header contains "Connection: close"

  // std::istringstream iss(msg);
  // std::vector<std::string> parsed
  //   ((std::istream_iterator<std::string>(iss)), (std::istream_iterator<std::string>()));
  // std::cout << parsed[1] << std::endl;
  
  HttpResponse response;


  response.set_error_code_(parser.get_error_code());
  
  if (showRequest)
    std::cout << "the resource path is " << parser.get_resource_path() << std::endl;
  response.assign_cont_type_(parser.get_resource_path()); 
  response.open_file(parser.get_resource_path());
  response.compose_header();
  if (showResponse)
  {
    std::cout << "\n------response header------" << std::endl;
    std::cout << response.get_header_() << std::endl;
    std::cout << "-----end of response header------\n" << std::endl;
  }
  send_to_client(clientSocket, response.get_header_().c_str(), response.get_header_().size());

  send_chunked_response(clientSocket, response.get_file_());
}

void VirtualHost::send_chunked_response(int clientSocket, std::ifstream &file)
{
  const int chunk_size = 1024;
   
  char buffer[chunk_size]{};
  // int i = 0;
  /* TODO: check if the handling of SIGINT on send error is needed  
    It would sigint on too many failed send() attempts*/
  if (file.is_open()){
    std::streamsize bytesRead;
    
    while (file) {
      file.read(buffer, chunk_size);
      bytesRead = file.gcount(); 
      if (bytesRead == -1)
      {
        std::cout << "bytesRead returned -1" << std::endl;
        break;
      }
      std::ostringstream chunk_size_hex;
      chunk_size_hex << std::hex << bytesRead << "\r\n";
      if (send_to_client(clientSocket, chunk_size_hex.str().c_str(), chunk_size_hex.str().length()) == -1 || 
        send_to_client(clientSocket, buffer, bytesRead) == -1 ||
        send_to_client(clientSocket, "\r\n", 2) == -1){
          perror("send :");
          break ;
      }
      // i ++;
    }
    // std::cout << "sent a chunk " << i << " times and the last one was " << bytesRead << std::endl;
    if (send_to_client(clientSocket, "0\r\n\r\n", 5) == -1)
      perror("send 2:");
  }
  else
    if (send_to_client(clientSocket, "<h1>404 Not Found</h1>", 23) == -1)
      perror("send 3:");
  file.close();
  std::cout << "\n-----response sent-----\n" << std::endl;
}

int VirtualHost::send_to_client(const int clientSocket, const char *msg, int length){
  return (send(clientSocket, msg, length, 0));
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
