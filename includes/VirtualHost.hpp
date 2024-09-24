/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualHost.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 17:08:10 by  dshatilo         #+#    #+#             */
/*   Updated: 2024/09/24 17:41:59 by klukiano         ###   ########.fr       */
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

#include "Location.hpp"
#include "HttpResponse.hpp"
#include "HttpParser.hpp"


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
  void        on_message_recieved(const int clientSocket, HttpParser &parser, pollfd &sock);

 private:
  std::string                        name_;
  std::map<std::string, std::string> error_pages_;
  size_t                             client_max_body_size_ = 1048576;
  std::map<std::string, Location>    locations_;

  int   send_to_client(const int clientSocket, const char *msg, int length);
  void  send_chunked_body(int clientSocket, std::ifstream &file,
    std::string resourcePath, pollfd &sock);
  int   send_one_chunk(int client_socket, std::ifstream &file, std::string resourcePath);
};

#endif
