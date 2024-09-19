/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 12:55:06 by dshatilo          #+#    #+#             */
/*   Updated: 2024/09/19 16:19:47 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket(std::string& socket, VirtualHost& v) : v_hosts_({{v.get_name(), v}}) {
  size_t colon = socket.find(':');
  address_ = socket.substr(0, colon);
  port_ = socket.substr(colon + 1);
}

std::string Socket::get_socket() {
  return address_ + ":" + port_;
}

void  Socket::add_virtual_host(VirtualHost& v) {
  v_hosts_[v.get_name()] = v;
}