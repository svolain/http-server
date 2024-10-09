/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientInfo.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  dshatilo < dshatilo@student.hive.fi >     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:39:21 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/08 23:03:24 by  dshatilo        ###   ########.fr       */
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
  char  buf[MAXBYTES]{0};
  int   bytesIn;

  bytesIn = recv(fd_, buf, MAXBYTES, 0);
  if (bytesIn < 0)
    return 1;
  if (bytesIn == 0) //When a stream socket peer has performed an orderly shutdown, the return value will be 0 (the traditional "end-of-file" return)
    return 2;
  logDebug("request is:\n" + std::string(buf), 1);
  if (!is_parsing_body_) {
    is_parsing_body_ = true;
    if (parser_.ParseHeader(buf)) {
      // poll.events = POLLOUT; //Error in header. Server can send error response skipping reading body part
      return 0;
    }
    vhost_ = sock_->FindVhost(parser_.getHost());
    //check if body size it too larg here?
  } else {
    // parser_.ParseBody();
    return vhost_->WriteBody(*this, poll);
  }
  return 0;
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

std::ofstream& ClientInfo::getPostfile() {
  return postfile_;
}

bool ClientInfo::getIsSending() {
  return is_sending_chunks_;
}

bool ClientInfo::getIsParsingBody() {
  return is_parsing_body_;
}

bool ClientInfo::getIsFileOpened() {
  return is_file_opened_;
}

void ClientInfo::setIsFileOpened(bool boolean) {
  is_file_opened_ = boolean;
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

std::ifstream& ClientInfo::getGetfile() {
  return getfile_;
}