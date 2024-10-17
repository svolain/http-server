/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 15:51:16 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/17 09:57:36 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP_
#define HTTPRESPONSE_HPP_

#include <iostream>
#include <map>
#include <fstream>
#include <sstream>
#include <poll.h>

class ClientConnection;

class HttpResponse {
 public:
  HttpResponse(std::string& status);
  HttpResponse(const HttpResponse& other)             = delete;
  HttpResponse& operator=(const HttpResponse& other)  = delete;


  ~HttpResponse() = default;

  void  CreateResponse(ClientConnection& fd_info, pollfd& poll);
  void  ResetResponse(); //DEFINE IT OR REMOVE

 private:
  void  AssignContType(std::string resourcePath);
  void  OpenFile(std::string& resource_path, std::ifstream& file);
  void  ComposeHeader();
  // std::string     getContType() const;
  // std::string     getResponseHeader() const;
  // std::string     getErrorCodeMessage() const;
  void InitContMap();
  void LookupStatusMessage();
  void SendHeader(ClientConnection& fd_info);
  void SendChunkedBody(ClientConnection& fd_info, pollfd &poll);
  int  SendOneChunk(int client_socket, std::ifstream &file);
  int  SendToClient(const int clientSocket, const char *msg, int length);

  std::string&                        status_;
  std::map<std::string, std::string>  cont_type_map_;
  std::string                         cont_type_;
  std::string                         status_message_;
  std::string                         header_;
};

#endif