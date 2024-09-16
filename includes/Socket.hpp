/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/10 13:34:24 by dshatilo          #+#    #+#             */
/*   Updated: 2024/09/16 14:16:48 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* ************************************************************************** */

#ifndef SOCKET_HPP_
#define SOCKET_HPP_

#include "VirtualHost.hpp"


class Socket {
 public:
  Socket(std::string& socket, VirtualHost& v);
  Socket(const Socket& other)            = default;
  Socket& operator=(const Socket& other) = default;

  ~Socket() = default;

  std::string GetSocket();
  void        AddVirtualHost(VirtualHost& v);

 private:
  std::string                         address_;
  std::string                         port_;
  std::map<std::string, VirtualHost>  v_hosts_;

};

#endif
