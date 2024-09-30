/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/10 13:34:24 by dshatilo          #+#    #+#             */
/*   Updated: 2024/09/30 13:54:08 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* ************************************************************************** */

#ifndef SOCKET_HPP_
#define SOCKET_HPP_

#include "VirtualHost.hpp"
#include "HttpParser.hpp"


#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <vector>
#include <string.h> //memset


class Socket {
 public:
  Socket(std::string& socket, VirtualHost& v);
  Socket(const Socket& other)            = default;
  Socket& operator=(const Socket& other) = default;
  ~Socket() = default;

  std::string                         get_socket();
  pollfd                              get_listening() const;
  std::map<std::string, VirtualHost>& get_v_hosts();
  void                                AddVirtualHost(VirtualHost& v);
  int                                 InitServer(std::vector<pollfd> &pollFDs);

 private:
  std::string                         address_;
  std::string                         port_;
  pollfd                              listening_;
  std::map<std::string, VirtualHost>  v_hosts_;

};

#endif
