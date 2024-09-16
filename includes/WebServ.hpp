/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/05 15:17:45 by shatilovdr        #+#    #+#             */
/*   Updated: 2024/09/16 12:35:17 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WebServ_HPP_
#define WebServ_HPP_

#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include "Location.hpp"
#include "VirtualHost.hpp"
#include "Socket.hpp"

#define DEFAULT_CONF "./conf/default.conf"

class WebServ {
 public:
  WebServ(char* config_file);
  WebServ(const WebServ& other)            = delete;
  WebServ& operator=(const WebServ& other) = delete;

  ~WebServ() = default;

  void  Run();
  int   Init();

 private:
  int ParseConfig();
  void ParseServer(std::stringstream& ss);
  void ParseLocation(VirtualHost& v, std::stringstream& ss);
  void ParseSocket(std::string& s, std::stringstream& ss);
  void ParseName(VirtualHost& v, std::stringstream& ss);
  void ParseMaxBodySize(VirtualHost& v, std::stringstream& ss);
  void ParseErrorPage(VirtualHost& v, std::stringstream& ss);
  void ParseAllowedMethods(Location& l, std::stringstream& ss);
  void ParseRedirection(Location& l, std::stringstream& ss);
  void ParseRoot(Location& l, std::stringstream& ss);
  void ParseAutoindex(Location& l, std::stringstream& ss);
  void ParseIndex(Location& l, std::stringstream& ss);

  std::string          conf_;
  std::deque<Socket>   sockets_;
  // std::vector<pollfd>  sockets_fd_;
};
#endif
