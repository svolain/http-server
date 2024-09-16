/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 18:09:41 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/16 18:28:13 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "WebServer.hpp"
#include "HttpResponse.hpp"
#include "HttpParser.hpp"

#include <functional>


extern bool showResponse;
extern bool showRequest;


WebServer::WebServer(const char *m_ipAddress, const char *m_port)
	: TcpListener(m_ipAddress, m_port){
	;
}

void WebServer::onMessageRecieved(const int clientSocket, const char *msg, 
  int bytesIn, short revents){
	
  (void)bytesIn;
  (void)revents;
  std::cout << "--- entering onMessageRecieved ---" << std::endl;
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
  std::cout << "\nrequestBody:\n" << parser.getrequestBody() << std::endl;

  std::cout << "the err code is " << parser.getErrorCode() << std::endl;
  response.setErrorCode(parser.getErrorCode());
  
  if (showRequest)
    std::cout << "the resource path is " << parser.getResourcePath() << std::endl;
  response.assignContType(parser.getResourcePath()); 
  response.openFile(parser.getResourcePath());
  response.composeHeader();
  if (showResponse)
  {
    std::cout << "\n------response header------" << std::endl;
    std::cout << response.getHeader() << std::endl;
    std::cout << "-----end of response header------\n" << std::endl;
  }
  sendToClient(clientSocket, response.getHeader().c_str(), response.getHeader().size());

  sendChunkedResponse(clientSocket, response.getFile());
}

void WebServer::sendChunkedResponse(int clientSocket, std::ifstream &file)
{
  const int chunk_size = 1024;
   
  char buffer[chunk_size]{};
  int i = 0;
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
      if (sendToClient(clientSocket, chunk_size_hex.str().c_str(), chunk_size_hex.str().length()) == -1 || 
        sendToClient(clientSocket, buffer, bytesRead) == -1 ||
        sendToClient(clientSocket, "\r\n", 2) == -1){
          perror("send :");
          break ;
      }
      i ++;
    }
    // std::cout << "sent a chunk " << i << " times and the last one was " << bytesRead << std::endl;
    if (sendToClient(clientSocket, "0\r\n\r\n", 5) == -1)
      perror("send 2:");
  }
  else
    if (sendToClient(clientSocket, "<h1>404 Not Found</h1>", 23) == -1)
      perror("send 3:");
  file.close();
  std::cout << "\n-----response sent-----\n" << std::endl;
}

void WebServer::onClientConnected()
{
	;
}

void WebServer::onClientDisconected()
{
	;
}