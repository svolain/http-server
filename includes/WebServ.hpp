/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/05 15:17:45 by shatilovdr        #+#    #+#             */
/*   Updated: 2024/09/24 17:54:16 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP_
#define WEBSERV_HPP_

#include <netdb.h> 
#include <unistd.h>
#include <poll.h>
#include <deque>
#include "Socket.hpp"
#include "ConnectInfo.hpp"

#define DEFAULT_CONF "./conf/default.conf"

class WebServ {
 public:
  WebServ(const char* config_file);
  WebServ(const WebServ& other)            = delete;
  WebServ& operator=(const WebServ& other) = delete;

  ~WebServ() = default;

  void  run();
  int   init();

 private:

  std::string                   conf_;
  std::deque<Socket>            sockets_;
  std::vector<pollfd>           pollFDs_;
  // std::map<int, Socket*>     connection_map;
  std::map<int, ConnectInfo>    connection_map;
  
  
  void poll_available_fds(void);
  bool is_sock_listening(int sock, short revents);
  void close_connection(int sock, int i);
  void close_all_connections(void);

  struct EventFlag {
      short flag;
      const char* description;
  };
  
};
#endif
