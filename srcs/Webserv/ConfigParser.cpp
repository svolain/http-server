/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/19 15:55:31 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/01 22:55:03 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigParser.hpp"
#include "WebServ.hpp"

ConfigParser::ConfigParser(const char* conf) : conf_(conf != nullptr ? conf : DEFAULT_CONF) {
}

int ConfigParser::ParseConfig(std::deque<Socket> &sockets_) {
  logInfo("Opening Conf file: " + conf_);
  std::ifstream input(conf_);
  if (!input.is_open()) {
    logError("Can't open input file: \"" + conf_ + "\"");
    return 1;
  }
  std::stringstream ss;
  while (!input.eof()) {
    std::string line;
    std::getline(input, line);
    if (size_t pos = line.find('#'); pos != std::string::npos)
      line.resize(pos);
    ss << line << '\n';
  }
  while (!ss.eof()) {
    try {
      ParseServer(ss, sockets_);
    } catch (std::string& error_token) {
        logError("invalid input: \"" + error_token + "\"");
        return 1;
    }
  }
  return 0;
}

void ConfigParser::ParseServer(std::stringstream& ss, std::deque<Socket> &sockets_) {
  std::string token;
  ss >> token;

  if (token == "")
    return;
  if (token != "server")
    throw token;
  ss >> token;
  if (token != "{")
    throw token;

  VirtualHost virtual_host;
  std::string                        listen;

  while (true) {
    ss >> token;
    if (token == "listen")
      ParseListen(listen, ss);
    else if (token == "server_name")
      ParseServerName(virtual_host, ss);
    else if (token == "client_max_body_size")
      ParseMaxBodySize(virtual_host, ss);
    else if (token == "error_page")
      ParseErrorPage(virtual_host, ss);
    else if (token == "location")
      ParseLocation(virtual_host, ss);
    else if (token == "}")
      break;
    else
      throw token;
  }
auto it = std::find_if(sockets_.begin(), sockets_.end(), [&](Socket& obj) {
      return obj.get_socket() == listen;
  });
  if (it != sockets_.end())
  {
    (*it).AddVirtualHost(virtual_host);
    /* Why it doesnt assign a name with the default config? */
  }
    
  else
    sockets_.push_back(Socket(listen, virtual_host));
  
}

void ConfigParser::ParseLocation(VirtualHost& v, std::stringstream& ss) {
  static std::regex path_format("/([a-zA-Z0-9_-]*/)*[a-zA-Z0-9_-]+");
  std::string       path;
  Location          location;
  std::string       token;
  ss >> path;
  if (!std::regex_match(path, path_format))
    throw "location " + path;
  ss >> token;
  if (token != "{")
    throw token;
  while (true) { //what if location is in format location / {} ?
    ss >> token;
    if (token == "limit_except")
      ParseAllowedMethods(location, ss);
    else if (token == "return")
      ParseRedirection(location, ss);
    else if (token == "root")
      ParseRoot(location, ss);
    else if (token == "autoindex")
      ParseAutoindex(location, ss);
    else if (token == "index")
      ParseIndex(location, ss);
    else if (token == "}")
      break;
    else
      throw token;
  }
   v.set_location(path, location);
}

void ConfigParser::ParseListen(std::string& socket, std::stringstream& ss) {
  static std::regex format("((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\\.)"
                           "{3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9]):"
                           "(6553[0-5]|655[0-2][0-9]|65[0-4][0-9]{2}|6[0-4]"
                           "[0-9]{3}|[1-5][0-9]{4}|[1-9][0-9]{1,3}|[0-9]);");
  ss >> socket;
  if (!std::regex_match(socket, format))
    throw "listen " + socket;
  socket.pop_back();
}

void ConfigParser::ParseServerName(VirtualHost& v, std::stringstream& ss) {
  static std::regex format(R"(([a-z0-9-]{1,63}\.){1,124}com;)");

  std::string token;
  ss >> token;
  if (!std::regex_match(token, format) || token.size() > 253)
    throw "server_name " + token;
  token.pop_back();
  v.set_name(token);
}

void ConfigParser::ParseMaxBodySize(VirtualHost& v, std::stringstream& ss) {
  std::regex format("0;|((0|[1-9][0-9]{0,5}|1000000)K;)"
                      "|((0|[1-9][0-9]{0,2}|1000)M;)");

  std::string token;
  ss >> token;
  if (!std::regex_match(token, format))
    throw "client_max_body_size " + token;
  token.pop_back();
  v.set_size(token);
}

void ConfigParser::ParseErrorPage(VirtualHost& v, std::stringstream& ss) {
  static std::regex code_format("404|505"); //input all possible error codes here
  static std::regex path_format("/([a-zA-Z0-9_-]*/)*[a-zA-Z0-9_-]+\\.html;");

  std::string code;
  std::string path;
  ss >> code;
  ss >> path;
  if (!std::regex_match(code, code_format)
      || !std::regex_match(path, path_format))
    throw "error_page " + code + " " + path;
  path.pop_back();
  v.set_error_page(code, path);
}

void ConfigParser::ParseAllowedMethods(Location& l, std::stringstream& ss) {
  static std::regex format("\\s*((GET|HEAD|POST|DELETE)\\s+)*"
                                "(GET|HEAD|POST|DELETE)\\s*"); 
  std::string line;
  std::getline(ss, line);
  if (!std::regex_match(line, format))
    throw "limit_except " + line;
  l.set_allowed_methods(line);
}

void ConfigParser::ParseRedirection(Location& l, std::stringstream& ss) {
  static std::regex code_format("30[0-478]");
  static std::regex path_format("(/([a-zA-Z0-9_-]*/)*[a-zA-Z0-9_-]+\\.html)|"
                                "((https?://)?(www\\.)?([a-zA-Z0-9_-]+\\.)+[a-zA-Z]{2,});");
  std::string code;
  std::string path;
  ss >> code;
  ss >> path;
  if (!std::regex_match(code, code_format)
      || !std::regex_match(path, path_format))
    throw "return " + code + " " + path;
  path.pop_back();
  l.set_redirection(code, path);
}

void ConfigParser::ParseRoot(Location& l, std::stringstream& ss) {
  static std::regex format("(/[a-zA-Z0-9_-]+)+;"); 
  std::string token;
  ss >> token;
  if (!std::regex_match(token, format))
    throw "root " + token;
  token.pop_back();
  l.set_root(token);
}

void ConfigParser::ParseAutoindex(Location& l, std::stringstream& ss) {
  static std::regex format("(on|off);"); 
  std::string token;
  ss >> token;
  if (!std::regex_match(token, format))
    throw "autoindex " + token;
  token.pop_back();
  l.set_auto_index(token);
}

void ConfigParser::ParseIndex(Location& l, std::stringstream& ss) {
  static std::regex format("[a-zA-Z0-9_-]+\\.html;"); 
  std::string token;
  ss >> token;
  if (!std::regex_match(token, format))
    throw "index " + token;
  token.pop_back();
  l.set_index(token);
}
