/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientInfo.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vsavolai <vsavolai@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:39:21 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/10 12:46:27 by vsavolai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientInfo.hpp"
#include "Socket.hpp"
#include "Logger.h"
#include <vector>

// ClientInfo::ClientInfo(int fd, Socket* sock)
//   : fd_(fd), sock_(sock), vhost_(nullptr), is_sending_chunks_(false) {}

void ClientInfo::InitInfo(int fd, Socket *sock) {
  fd_ = fd;
  sock_ = sock;
}

int ClientInfo::RecvRequest(pollfd& poll) {
  std::vector<char> buffer(MAXBYTES);
  int               bytesIn;

  bytesIn = recv(fd_, buffer.data(), MAXBYTES, 0);
  if (bytesIn < 0)
    return 1;
  if (bytesIn == 0) //When a stream socket peer has performed an orderly shutdown, the return value will be 0 (the traditional "end-of-file" return)
    return 2;
  logDebug("request is:\n" + std::string(buffer.data()), 1);
  if (!is_parsing_body_) {
    bool header_parsed = parser_.ParseHeader(buffer.data());
    vhost_ = sock_->FindVhost(parser_.getHost());
    if (!header_parsed || parser_.getMethod() == "GET"
        || parser_.getMethod() == "HEAD"|| !parser_.IsBodySizeValid(vhost_)) {
      poll.events = POLLOUT; //Error in header. Server can send error response skipping reading body part
    }
  } else if (parser_.WriteBody(vhost_, buffer, bytesIn)){
    poll.events = POLLOUT;
  }
  return 0;
}

HttpParser& ClientInfo::getParser() {
  return parser_;
}

VirtualHost*  ClientInfo::getVhost() {
  return vhost_;
}

int ClientInfo::getFd() {
  return fd_;
}

bool ClientInfo::getIsSending() {
  return is_sending_chunks_;
}

std::ifstream& ClientInfo::getGetfile() {
  return getfile_;
}

void ClientInfo::setIsSending(bool boolean) {
  is_sending_chunks_ = boolean;
}
