/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/06 15:30:31 by dshatilo          #+#    #+#             */
/*   Updated: 2024/09/20 15:57:15 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"
#include "HttpResponse.hpp"
#include "HttpParser.hpp"
#include "ConfigParser.hpp"

extern bool showResponse;
extern bool showRequest;

#define TODO 123

WebServ::WebServ(char* conf) : conf_(conf != nullptr ? conf : DEFAULT_CONF) {}



int WebServ::init() {

  {
    ConfigParser parser;
    if (parser.parse_config(this->sockets_))
      return 1;
  }
  
  int i = 0;
  for (auto it = sockets_.begin(); it != sockets_.end(); it ++, i ++)
  {
    if (sockets_[i].init_server())
      return 2;
  }
  return (0);
}



void WebServ::run() {
  std::cout << "Servers are ready.\n";

  int i;
  while (1)
  {
    for (auto it = sockets_.begin(); it != sockets_.end(); it++, i ++)
    { 
      //will we need to terminate a single server at any point?
      if (sockets_[i].poll_server())
      {
        sockets_[i].close_all_connections();
        sockets_.erase(it);
      }
    }
  }

  for (auto it = sockets_.begin(); it != sockets_.end(); it++, i ++)
    sockets_[i].close_all_connections();

  std::cout << "--- Shutting down the server ---" << std::endl;
}


void WebServ::on_message_recieved(const int clientSocket, const char *msg, 
  int bytesIn, short revents){
  
  (void)bytesIn;
  (void)revents;
  std::cout << "--- entering on_message_recieved ---" << std::endl;
  /* TODO if bytesIn == MAXBYTES then recv until the whole message is sent 
    see 100 Continue status message
    https://www.w3.org/Protocols/rfc2616/rfc2616-sec8.html#:~:text=Requirements%20for%20HTTP/1.1%20origin%20servers%3A*/

  //TODO: check if the header contains "Connection: close"

  // std::istringstream iss(msg);
  // std::vector<std::string> parsed
  //   ((std::istream_iterator<std::string>(iss)), (std::istream_iterator<std::string>()));
  // std::cout << parsed[1] << std::endl;
  
  HttpParser parser;
  HttpResponse response;


  if (!parser.parseRequest(msg))
    std::cout << "false on parseRequest returned" << std::endl;
  std::cout << "\nrequestBody:\n" << parser.get_request_body() << std::endl;

  std::cout << "the err code is " << parser.get_error_code() << std::endl;
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

void WebServ::send_chunked_response(int clientSocket, std::ifstream &file)
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

void WebServ::onClientConnected()
{
  ;
}

void WebServ::onClientDisconected()
{
  ;
}

int WebServ::send_to_client(const int clientSocket, const char *msg, int length){
  return (send(clientSocket, msg, length, 0));
}

