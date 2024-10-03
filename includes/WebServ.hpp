/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/05 15:17:45 by shatilovdr        #+#    #+#             */
/*   Updated: 2024/10/03 09:57:26 by dshatilo         ###   ########.fr       */
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

#define DEFAULT_CONF "conf/default.conf"

class WebServ {
 public:
  WebServ(const char* config_file);
  WebServ(const WebServ& other)            = delete;
  WebServ& operator=(const WebServ& other) = delete;

  ~WebServ() = default;

  void  run();
  int   init();

 private:
  const std::string             conf_;
  std::deque<Socket>            sockets_;
  std::vector<pollfd>           pollFDs_;
  std::map<int, ConnectInfo>    client_info_map_;
  
  void        PollAvailableFDs(void);
  void        CheckForNewConnection(int fd, short revents, int i);
  void        CloseConnection(int sock, size_t& i);
  void        CloseAllConnections(void);
  void        RecvFromClient(ConnectInfo* fd_info, size_t& i);
  void        SendToClient(ConnectInfo* fd_info, pollfd& poll);
  std::string ToString() const;

  struct EventFlag {
      short flag;
      const char* description;
  };
  
};
#endif
