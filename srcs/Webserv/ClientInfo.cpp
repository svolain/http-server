/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientInfo.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:39:21 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/07 08:33:54 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientInfo.hpp"
#include "Socket.hpp"
#include "Logger.h"

// ClientInfo::ClientInfo(int fd, Socket* sock)
//   : fd_(fd), sock_(sock), vhost_(nullptr), is_sending_chunks_(false) {}

void ClientInfo::InitInfo(int fd, Socket *sock) {
  fd_ = fd;
  sock_ = sock;
}

void ClientInfo::AssignVHost() {
  
  std::map<std::string, VirtualHost>&v_hosts_ = getSocket()->getVirtualHosts();
  std::map<std::string, std::string>&headers = parser_.getHeaders();
  std::map<std::string, VirtualHost>::iterator vhosts_it;
  try {
    /* Find the value of the Host key in the headers map */
    vhosts_it = v_hosts_.find(headers.at("Host"));
  } catch (const std::out_of_range& e) {
    logDebug("AssignVHost: map at() except: No host field in the provided header", true);
    vhosts_it = v_hosts_.end();
  }

  if (vhosts_it != v_hosts_.end()) {
    logDebug("Found requested host: " + vhosts_it->first);
    setVhost(&vhosts_it->second);
  }
  else {
    vhosts_it = v_hosts_.begin();
    setVhost(&vhosts_it->second);
  }
}

int ClientInfo::RecvRequest(pollfd& poll) {
  if (!is_parsing_body_)
    return vhost_->ParseHeader(*this, poll);
  else
    return vhost_->WriteBody(*this, poll);
}

void ClientInfo::setVhost(VirtualHost *vhost) {
  vhost_ = vhost;
}

void ClientInfo::setIsParsingBody(bool boolean) {
  is_parsing_body_ = boolean;
}

void ClientInfo::setIsSending(bool boolean) {
  is_sending_chunks_ = boolean;
}

bool ClientInfo::getIsSending() {
  return is_sending_chunks_;
}

VirtualHost*  ClientInfo::getVhost() {
  return vhost_;
}

HttpParser& ClientInfo::getParser() {
  return parser_;
}

Socket* ClientInfo::getSocket() {
  return sock_;
}

int ClientInfo::getFd() {
  return fd_;
}

std::ifstream& ClientInfo::getFile() {
  return getfile_;
}