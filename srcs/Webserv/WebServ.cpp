/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/06 15:30:31 by dshatilo          #+#    #+#             */
/*   Updated: 2024/09/22 16:54:05 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"
#include "ConfigParser.hpp"

#define TODO 123

WebServ::WebServ(const char* conf) : conf_(conf != nullptr ? conf : DEFAULT_CONF) {
  
}

int WebServ::init() {

  {
    ConfigParser parser(conf_.c_str());
    if (parser.parse_config(this->sockets_))
      return 1;
    
  }
  
  int i = 0;
  for (auto it = sockets_.begin(); it != sockets_.end(); it ++, i ++)
  {
    if (sockets_[i].init_server())
      return 2;
    std::cout << "init the server on socket " << sockets_[i].get_socket() << std::endl; 
  }
  return (0);
}



void WebServ::run() {
  std::cout << "Servers are ready.\n";

  int i;
  while (1)
  {
    i = 0;
    for (auto it = sockets_.begin(); it != sockets_.end(); it++, i ++)
    { 
      //will we need to terminate a single server at any point?
      if (sockets_[i].poll_server())
      {
        sockets_[i].close_all_connections();
        sockets_.erase(it);
      }
      std::cout << "poll next server" << std::endl;
    }
  }

  for (auto it = sockets_.begin(); it != sockets_.end(); it++, i ++)
    sockets_[i].close_all_connections();

  std::cout << "--- Shutting down the server ---" << std::endl;
}


