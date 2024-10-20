/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:39:21 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/19 17:16:35 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ClientConnection.hpp"
#include "CGIConnection.hpp"
#include "Socket.hpp"
#include "Logger.h"
#include <vector>

ClientConnection::ClientConnection(int fd, Socket& sock, WebServ& webserv)
    : Connection(fd, 20),
      sock_(sock),
      webserv_(webserv),
      parser_(status_),
      response_(status_)  {}

int ClientConnection::ReceiveData(pollfd& poll) {
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
    vhost_ = sock_.FindVhost(parser_.getHost());
    if (!header_parsed || !parser_.HandleRequest(vhost_))
      poll.events = POLLOUT;
    parser_.OpenFile(*this);
    is_parsing_body_ = true;
  } else if (parser_.WriteBody(vhost_, buffer, bytesIn))
      poll.events = POLLOUT;
  return 0;
}

int ClientConnection::SendData(pollfd& poll) {
  response_.SendResponse(*this, poll);
  if (poll.events == POLLIN)
    ResetClientConnection();
  (void)webserv_; //REMOVE_ME                                         
  return 0; //Add protection for send()!
}

void  ClientConnection::ResetClientConnection() {
  status_ = "200";
  vhost_ = nullptr;
  parser_.ResetParser();
  response_.ResetResponse();
  is_sending_chunks_ = false;
  is_parsing_body_ = false;
}

HttpParser& ClientConnection::getParser() {
  return parser_;
}

VirtualHost*  ClientConnection::getVhost() {
  return vhost_;
}

int ClientConnection::getFd() {
  return fd_;
}

bool ClientConnection::getIsSending() {
  return is_sending_chunks_;
}

std::fstream& ClientConnection::getGetfile() {
  return file_;
}

void ClientConnection::setIsSending(bool boolean) {
  is_sending_chunks_ = boolean;
}
