/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/19 15:56:49 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/30 12:44:29 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGPARSER_HPP_
#define CONFIGPARSER_HPP_

#include "Location.hpp"
#include "VirtualHost.hpp"
#include "Socket.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>


class ConfigParser
{
    public:
      ConfigParser() = default;
      ~ConfigParser() = default;
      ConfigParser(const char* conf);
      
      int ParseConfig(std::deque<Socket> &sockets_);

    private:
      void ParseServer(std::stringstream& ss, std::deque<Socket> &sockets_);
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
};

#endif

