/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientInfo.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:39:21 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/03 18:06:26 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientInfo.hpp"
#include "Socket.hpp"
#include "Logger.h"

ClientInfo::ClientInfo(int fd, Socket* sock)
  : fd_(fd), sock_(sock), vhost_(nullptr), is_sending_chunks_(false) {}

void ClientInfo::InitInfo(int fd, Socket *sock) {
  fd_ = fd;
  sock_ = sock;
  vhost_ = nullptr;
  is_sending_chunks_ = false;
}

void ClientInfo::AssignVHost() {
  std::map<std::string, VirtualHost>&v_hosts_ = get_socket()->get_v_hosts();
  std::map<std::string, std::string>&headers = parser_.get_headers();
  std::map<std::string, VirtualHost>::iterator vhosts_it;
  try {
    /* Find the value of the Host key in the headers map */
    vhosts_it = v_hosts_.find(headers.at("Host"));
  }
  catch(const std::out_of_range& e) {
    logDebug("AssignVHost: map at() except: No host field in the provided header", true);
    vhosts_it = v_hosts_.end();
  }
  
  if (vhosts_it != v_hosts_.end()){
    logDebug("Found requested host: " + vhosts_it->first);
    set_vhost(&vhosts_it->second);
  }
  else {
    vhosts_it = v_hosts_.begin();
    set_vhost(&vhosts_it->second);
  }
}

void ClientInfo::set_vhost(VirtualHost *vhost) {
  vhost_ = vhost;
}

void ClientInfo::set_is_parsing_body(bool boolean) {
  is_parsing_body_ = boolean;
}

void ClientInfo::set_is_sending(bool boolean) {
  is_sending_chunks_ = boolean;
}

bool ClientInfo::get_is_sending() {
  return is_sending_chunks_;
}

bool ClientInfo::get_is_parsing_body() {
  return is_parsing_body_;
}

VirtualHost*  ClientInfo::get_vhost() {
  return vhost_;
}

HttpParser* ClientInfo::get_parser() {
  return &parser_;
}

Socket* ClientInfo::get_socket() {
  return sock_;
}

int ClientInfo::get_fd() {
  return fd_;
}

std::ifstream& ClientInfo::get_file() {
  return getfile_;
}