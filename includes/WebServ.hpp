/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/05 15:17:45 by shatilovdr        #+#    #+#             */
/*   Updated: 2024/10/17 16:30:42 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP_
#define WEBSERV_HPP_

#include <memory>
#include <netdb.h>
#include <unistd.h>
#include <poll.h>
#include <deque>
#include "Socket.hpp"
#include "Connection.hpp"

#define DEFAULT_CONF "conf/default.conf"

class WebServ {
 public:
  WebServ(const char* config_file);
  WebServ(const WebServ& other)            = delete;
  WebServ& operator=(const WebServ& other) = delete;

  ~WebServ() = default;

  int   Init();
  void  Run();

 private:
  void        PollAvailableFDs();
  void        CheckForNewConnection(int fd, short revents, int i);
  void        ReceiveData(Connection& connection, size_t& i);
  void        SendData(Connection& connection, pollfd& poll);
  void        CloseConnection(int sock, size_t& i);
  void        CloseAllConnections();
  std::string ToString() const;

  const std::string                          conf_;
  std::deque<Socket>                         sockets_;
  std::vector<pollfd>                        pollFDs_;
  std::map<int, std::unique_ptr<Connection>> connections_;
};
#endif
