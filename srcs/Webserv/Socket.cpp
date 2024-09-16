/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 12:55:06 by dshatilo          #+#    #+#             */
/*   Updated: 2024/09/16 14:21:35 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"

Socket::Socket(std::string& socket, VirtualHost& v) : v_hosts_({{v.GetName(), v}}) {
  size_t colon = socket.find(':');
  address_ = socket.substr(0, colon);
  port_ = socket.substr(colon + 1);
}

std::string Socket::GetSocket() {
  return address_ + ":" + port_;
}

void  Socket::AddVirtualHost(VirtualHost& v) {
  v_hosts_[v.GetName()] = v;
}