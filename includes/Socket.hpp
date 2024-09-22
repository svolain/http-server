/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/10 13:34:24 by dshatilo          #+#    #+#             */
/*   Updated: 2024/09/22 17:33:06 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* ************************************************************************** */

#ifndef SOCKET_HPP_
#define SOCKET_HPP_

#include "VirtualHost.hpp"
#include "HttpParser.hpp"


#include <unistd.h>
#include <poll.h>
#include <vector>
#include <fcntl.h>
#include <string.h> //memset


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

  std::map<std::string, VirtualHost>  v_hosts_;
  
 private:
  std::string                         address_;
  std::string                         port_;
  // std::map<std::string, VirtualHost>  v_hosts_;

  pollfd                              listening_;
  std::vector<pollfd>                 pollFDs_;

};

#endif
