/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualHost.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 17:08:10 by  dshatilo         #+#    #+#             */
/*   Updated: 2024/10/02 17:08:51 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VIRTUALHOST_HPP_
#define VIRTUALHOST_HPP_

#include <iostream>
#include <string>
#include <map>
#include <utility>
#include <netdb.h> 
#include <poll.h>
#include <vector>

#include "Location.hpp"
#include "HttpResponse.hpp"
#include "HttpParser.hpp"

class ConnectInfo;

class VirtualHost {
 private:
  using StringMap = std::map<std::string, std::string>;
  using LocationMap = std::map<std::string, Location>;
  using StringPair = std::pair<std::string, std::string>;

 public:
  VirtualHost()                                    = default;
  VirtualHost(std::string& max_size, StringMap& errors, LocationMap& locations);
  VirtualHost(const VirtualHost& other)            = default;
  VirtualHost& operator=(const VirtualHost& other) = default;
  ~VirtualHost() = default;

  size_t      get_max_body_size();
  int         ParseHeader(ConnectInfo* fd_info, pollfd& poll);
  int         WriteBody(ConnectInfo* fd_info, pollfd& poll);
  void        OnMessageRecieved(ConnectInfo *fd_info, pollfd &poll);
  
 private:
  std::string name_;
  StringMap   error_pages_;
  size_t      client_max_body_size_ = 1048576;
  LocationMap locations_;
  
  void  SendHeader(ConnectInfo *fd_info);
  void  SendChunkedBody(ConnectInfo* fd_info, pollfd &poll);
  int   SendOneChunk(int client_socket, std::ifstream &file);
  int   SendToClient(const int clientSocket, const char *msg, int length);
  
};

#endif
