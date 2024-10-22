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
#include "CgiConnection.hpp"
#include "Socket.hpp"
#include "Logger.hpp"
#include <vector>

ClientConnection::ClientConnection(int fd, Socket& sock, WebServ& webserv)
    : Connection(fd, 20),
      sock_(sock),
      webserv_(webserv),
      parser_(*this),
      response_(*this) {}

int ClientConnection::ReceiveData(pollfd& poll) {
  std::vector<char> buffer(MAXBYTES);
  int               bytesIn;
  bytesIn = recv(fd_, buffer.data(), MAXBYTES, 0);
  if (bytesIn < 0)
    return 1;
  if (bytesIn == 0)
    return 2;
  logDebug("request is:\n", buffer.data());

  if (stage_ == Stage::kHeader) {
    bool header_parsed = parser_.ParseHeader(buffer.data());
    vhost_ = sock_.FindVhost(parser_.getHost());
    if (!header_parsed || !parser_.HandleRequest()) {
      file_.open(vhost_->getErrorPage(status_));
      stage_ = Stage::kResponse;
    }
  }
  if (stage_ == Stage::kBody) {
    bool body_read = parser_.WriteBody(buffer, bytesIn); //not sure that it's correct
    if (!body_read) {
      file_.open(vhost_->getErrorPage(status_));
      stage_ = Stage::kResponse;
    }
  }
  if (stage_ == Stage::kCgi) {
    ;//waitpid
  }
  if (stage_ == Stage::kResponse)
    poll.events = POLLOUT;
  return 0;
}

int ClientConnection::SendData(pollfd& poll) {
  if (response_.SendResponse(poll)) {
    file_.close();
    return 1;
  } 
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
  stage_ = Stage::kHeader;
}

bool ClientConnection::getIsSending() {
  return is_sending_chunks_;
}

void ClientConnection::setIsSending(bool boolean) {
  is_sending_chunks_ = boolean;
}
