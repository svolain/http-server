/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/06 15:30:31 by dshatilo          #+#    #+#             */
/*   Updated: 2024/09/16 14:40:17 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

WebServ::WebServ(char* conf) : conf_(conf != nullptr ? conf : DEFAULT_CONF) {}

void WebServ::Run() {
  std::cout << "Server is ready.\n";

}

int WebServ::Init() {
  if (ParseConfig()) 
    return 1;
  return 0;
}

int WebServ::ParseConfig() {
  std::cout << "[INFO] Opening Conf file: " << conf_ <<'\n';
  std::ifstream input(conf_);
  if (!input.is_open()) {
    std::cerr << "[ERROR] Can't open input file: \"" << conf_ << "\"\n";
    return 1;
  }
  std::stringstream ss;
  while (!input.eof()) {
    std::string line;
    std::getline(input, line);
    if (size_t pos = line.find('#') != std::string::npos)
       line.resize(pos);
    ss << line << ' ';
  }
  while (!ss.eof()) {
    try {
      ParseServer(ss);
    } catch (std::string& error_token) {
      std::cerr << "[ERROR] invalid input: \"" << error_token << "\"\n";
      return 1;
    }
  }
  return 0;
}

void WebServ::ParseServer(std::stringstream& ss) {
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
    std::string socket;
    while (true) {
      ss >> token;
      if (token == "listen")
        ParseSocket(socket, ss);
      else if (token == "server_name")
        ParseName(virtual_host, ss);
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
        return obj.GetSocket() == socket;
    });
  if (it != sockets_.end())
    (*it).AddVirtualHost(virtual_host);
  else
    sockets_.push_back(Socket(socket, virtual_host));
}

void WebServ::ParseLocation(VirtualHost& v, std::stringstream& ss) {
  static std::regex path_format("/[a-zA-Z0-9_-]*");
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
   v.SetLocation(path, location);
}

void WebServ::ParseSocket(std::string& socket, std::stringstream& ss) {
  static std::regex format("((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\\.)"
                           "{3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9]):"
                           "(6553[0-5]|655[0-2][0-9]|65[0-4][0-9]{2}|6[0-4]"
                           "[0-9]{3}|[1-5][0-9]{4}|[1-9][0-9]{1,3}|[0-9]);");
  ss >> socket;
  if (!std::regex_match(socket, format))
    throw "listen " + socket;
  socket.pop_back();
}

void WebServ::ParseName(VirtualHost& v, std::stringstream& ss) {
  static std::regex format(R"(([a-z0-9-]{1,63}\.){1,124}com;)");
  std::string token;
  ss >> token;
  if (!std::regex_match(token, format) || token.size() > 253)
    throw "server_name " + token;
  token.pop_back();
  v.SetName(token);
}

void WebServ::ParseMaxBodySize(VirtualHost& v, std::stringstream& ss) {
  static std::regex format("0|[1-9][0-9]{0,5}|1000000)K"
                           "|(0|[1-9][0-9]{0,2}|1000)M);");
  std::string token;
  ss >> token;
  if (!std::regex_match(token, format))
    throw "client_max_body_size " + token;
  token.pop_back();
  v.SetSize(token);
}

void WebServ::ParseErrorPage(VirtualHost& v, std::stringstream& ss) {
  static std::regex code_format("404|505"); //all possible error codes
  static std::regex path_format("(\\.{1,2}/)?([a-zA-Z0-9_-]+/)*"
                                "[a-zA-Z0-9_-]+\\.html;");
  std::string code;
  std::string path;
  ss >> code;
  ss >> path;
  if (!std::regex_match(code, code_format)
      || !std::regex_match(code, path_format))
    throw "error_page " + code + " " + path;
  path.pop_back();
  v.SetErrorPage(code, path);
}

void WebServ::ParseAllowedMethods(Location& l, std::stringstream& ss) {
  static std::regex format("\\s*((GET|HEAD|POST|DELETE)\\s+)*"
                                "(GET|HEAD|POST|DELETE)\\s*"); 
  std::string line;
  std::getline(ss, line, ';');
  if (!std::regex_match(line, format))
    throw "limit_except " + line;
  l.SetAllowedMethods(line);
}

void WebServ::ParseRedirection(Location& l, std::stringstream& ss) {
  static std::regex code_format("30[0-478]");
  static std::regex path_format("(\\.{1,2}/)?([a-zA-Z0-9_-]/)*"
                                "[a-zA-Z0-9_-]+\\.html;"); //Not sure about allowed redirections
  std::string code;
  std::string path;
  ss >> code;
  ss >> path;
  if (!std::regex_match(code, code_format)
      || !std::regex_match(code, path_format))
    throw "return " + code + " " + path;
  path.pop_back();
  l.SetRedirection(code, path);
}

void WebServ::ParseRoot(Location& l, std::stringstream& ss) {
  static std::regex format("(/[a-zA-Z0-9_-]+)+;"); 
  std::string token;
  ss >> token;
  if (!std::regex_match(token, format))
    throw "root " + token;
  token.pop_back();
  l.SetRoot(token);
}

void WebServ::ParseAutoindex(Location& l, std::stringstream& ss) {
  static std::regex format("(on|off);"); 
  std::string token;
  ss >> token;
  if (!std::regex_match(token, format))
    throw "autoindex " + token;
  token.pop_back();
  l.SetAutoindex(token);
}

void WebServ::ParseIndex(Location& l, std::stringstream& ss) {
  static std::regex format("[a-zA-Z0-9_-]+\\.html;"); 
  std::string token;
  ss >> token;
  if (!std::regex_match(token, format))
    throw "index " + token;
  token.pop_back();
  l.SetIndex(token);
}