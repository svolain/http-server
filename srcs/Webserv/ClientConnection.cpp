/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:39:21 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/15 13:13:24 by dshatilo         ###   ########.fr       */
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
      response_(status_) {}

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
    if (!header_parsed || parser_.getMethod() == "GET"
        || parser_.getMethod() == "HEAD"|| !parser_.IsBodySizeValid(vhost_)) {
      poll.events = POLLOUT;
    }
  } else if (parser_.WriteBody(vhost_, buffer, bytesIn)){
    poll.events = POLLOUT;
  }
  bool cgi = false;
  if (cgi)
    
  return 0;
}

int ClientConnection::SendData(pollfd& poll) {
  response_.CreateResponse(*this, poll);
  if (poll.events == POLLIN)
    ResetClientConnection();
  (void)webserv_; //REMOVE_ME                                         
  return 0; //Add protection for send()!
}

void  ClientConnection::ResetClientConnection() {
  status_ = "200";
  //getfile_.close(); Remove it?
  vhost_ = nullptr;
  parser_.ResetParser();
  response_.ResetResponse();
  is_sending_chunks_ = false;
  is_parsing_body_ = false;
}

void  ClientConnection::CreateCGIConnection() {
  int pipe_fd[2];
  if (pipe(pipe_fd) == -1) {
    status_ = "500";
    logError("500 Internal Server Error");
  }

  pid_t id = fork();
  if (id == -1) {
    //error occured
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    status_ = "500";
    logError("500 Internal Server Error");
  }
  if (id != 0) {
    close(pipe_fd[1]);
    pollfd cgi_poll{0};
    cgi_poll.fd = pipe_fd[0];
    fcntl(cgi_poll.fd, F_SETFL, O_NONBLOCK);
    cgi_poll.events = POLLIN;
    webserv_.AddNewConnection(cgi_poll, std::make_unique<CGIConnection>());
    //wait?
    //set cgi_is_running status may be
  }
  else {
    //prepare env
    //call execve
    std::terminate();
  }
}

void  ClientConnection::CleanupConnection() {
  //                  somenthig here;
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

std::ifstream& ClientConnection::getGetfile() {
  return getfile_;
}

void ClientConnection::setIsSending(bool boolean) {
  is_sending_chunks_ = boolean;
}
