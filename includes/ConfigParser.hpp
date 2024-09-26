/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/19 15:56:49 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/21 14:45:12 by klukiano         ###   ########.fr       */
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
      
      int parse_config(std::deque<Socket> &sockets_);

    private:
      void parse_server(std::stringstream& ss, std::deque<Socket> &sockets_);
      void parse_location(VirtualHost& v, std::stringstream& ss);
      void parse_socket(std::string& s, std::stringstream& ss);
      void parse_name(VirtualHost& v, std::stringstream& ss);
      void parse_max_body_size(VirtualHost& v, std::stringstream& ss);
      void parse_error_page(VirtualHost& v, std::stringstream& ss);
      void parse_allowed_methods(Location& l, std::stringstream& ss);
      void parse_redirection(Location& l, std::stringstream& ss);
      void parse_root(Location& l, std::stringstream& ss);
      void parse_autoindex(Location& l, std::stringstream& ss);
      void parse_index(Location& l, std::stringstream& ss);

      std::string          conf_;
};

#endif

