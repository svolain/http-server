/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/10 13:34:24 by dshatilo          #+#    #+#             */
/*   Updated: 2024/09/20 15:57:01 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* ************************************************************************** */

#ifndef SOCKET_HPP_
#define SOCKET_HPP_

#include "VirtualHost.hpp"

#include <netdb.h> 
#include <unistd.h>
#include <poll.h>

class Socket {
 public:
  Socket(std::string& socket, VirtualHost& v);
  Socket(const Socket& other)            = default;
  Socket& operator=(const Socket& other) = default;
  ~Socket() = default;

  std::string                         get_socket();
  void                                add_virtual_host(VirtualHost& v);
  int                                 init_server();
  int                                 poll_server(void);
  void                                close_all_connections(void);
  
 private:
  std::string                         address_;
  std::string                         port_;
  std::map<std::string, VirtualHost>  v_hosts_;

  pollfd                              listening_;
  std::vector<pollfd>                 pollFDs_;
};

#endif
