/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualHost.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 17:08:10 by  dshatilo         #+#    #+#             */
/*   Updated: 2024/09/30 13:55:15 by klukiano         ###   ########.fr       */
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
 public:
  VirtualHost()                                    = default;
  VirtualHost(const VirtualHost& other)            = default;
  VirtualHost& operator=(const VirtualHost& other) = default;
  ~VirtualHost() = default;

  std::string get_name();
  size_t      get_max_body_size();
  void        set_name(std::string& name);
  void        set_size(std::string& size);
  void        set_error_page(std::string& code, std::string& path);
  void        set_location(std::string& path, Location& location);
  void        OnMessageRecieved(ConnectInfo *fd_info, pollfd &poll);
  
 private:
  std::string                        name_;
  std::map<std::string, std::string> error_pages_;
  size_t                             client_max_body_size_ = 1048576;
  std::map<std::string, Location>    locations_;
  
  void  SendHeader(ConnectInfo *fd_info);
  void  SendChunkedBody(ConnectInfo* fd_info, pollfd &poll);
  int   SendOneChunk(int client_socket, std::ifstream &file);
  int   SendToClient(const int clientSocket, const char *msg, int length);
  
};

#endif
