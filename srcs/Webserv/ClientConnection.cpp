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
  else if (stage_ == Stage::kBody) {
    bool body_read = parser_.WriteBody(buffer, bytesIn);
    if (!body_read) {
      file_.open(vhost_->getErrorPage(status_));
      stage_ = Stage::kResponse;
    }
  }
  else if (stage_ == Stage::kCgi) { //move it to response - reference invalidation!!
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
  file_.close();
  stage_ = Stage::kHeader;
}

std::vector<std::string>  ClientConnection::PrepareCgiEvniron() {
  std::vector<std::string>  env;
  env.push_back("REQUEST_METHOD=" + parser_.method_);
  env.push_back("QUERY_STRING=" + parser_.query_string_);
  env.push_back("SCRIPT_NAME=" + parser_.request_target_.substr(1));
  env.push_back("SERVER_NAME=" + vhost_->getName());
  // env.push_back("CONTENT_TYPE=" + vhost_->getName());
  // env.push_back("CONTENT_LENGTH=" + vhost_->getName());
  //env.push_back("PATH_INFO=" + std::filesystem::current_path());
  env.push_back("GATEWAY_INTERFACE=CGI/1.1");

  return env;
}

WebServ&  ClientConnection::getWebServ() {
  return webserv_;
}
std::fstream& ClientConnection::getFile() {
  return file_;
}

void  ClientConnection::setStatus(const std::string& status) {
  status_ = status;
}
