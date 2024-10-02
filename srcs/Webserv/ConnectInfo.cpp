/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectInfo.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:39:21 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/24 17:39:21 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectInfo.hpp"
#include "Socket.hpp"

void ConnectInfo::InitInfo(int fd, Socket *sock) {
  fd_ = fd;
  sock_ = sock;
  vhost_ = nullptr;
  is_sending_chunks_ = false;
}

void ConnectInfo::AssignVHost() {
  std::map<std::string, VirtualHost>*v_hosts_ = &get_socket()->get_v_hosts();
  std::map<std::string, std::string>&headers = parser_.get_headers();
  std::map<std::string, VirtualHost>::iterator vhosts_it = (*v_hosts_).find(headers.at("Host"));
  if (vhosts_it != (*v_hosts_).end()){
    std::cout << "found " << vhosts_it->second.get_name() << std::endl;
    set_vhost(&vhosts_it->second);
  }
  else {
    vhosts_it = (*v_hosts_).begin();
    set_vhost(&vhosts_it->second);
  }
}

void ConnectInfo::set_vhost(VirtualHost *vhost) {
  vhost_ = vhost;
}

void ConnectInfo::set_is_parsing_body(bool boolean) {
  is_parsing_body_ = boolean;
}

void ConnectInfo::set_is_sending(bool boolean) {
  is_sending_chunks_ = boolean;
}

bool ConnectInfo::get_is_sending() {
  return is_sending_chunks_;
}

bool ConnectInfo::get_is_parsing_body() {
  return is_parsing_body_;
}

VirtualHost*  ConnectInfo::get_vhost() {
  return vhost_;
}

HttpParser* ConnectInfo::get_parser() {
  return &parser_;
}

Socket* ConnectInfo::get_socket() {
  return sock_;
}

int ConnectInfo::get_fd() {
  return fd_;
}

std::ifstream& ConnectInfo::get_file() {
  return getfile_;
}