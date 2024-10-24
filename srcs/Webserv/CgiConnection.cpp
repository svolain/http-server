/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiConnection.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/18 14:07:41 by dshatilo          #+#    #+#             */
/*   Updated: 2024/10/24 13:05:31 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <unistd.h>
#include "CgiConnection.hpp"
#include "Logger.hpp"
#include <signal.h>

#define READ 0
#define WRITE 1

CgiConnection::CgiConnection(int pipe_fd[2], ClientConnection& client,
                             pid_t child_pid)
    : Connection(pipe_fd[WRITE], 10),
      client_(client),
      webserv_(client_.getWebServ()),
      file_(client.getFile()),
      child_pid_(child_pid) {
  pipe_fd_[READ] = pipe_fd[READ];
  pipe_fd_[WRITE] = pipe_fd[WRITE];
  }

CgiConnection::~CgiConnection() {
  close(fd_);
  kill(child_pid_, SIGTERM);
  if (HasTimedOut()) {
    //something
  }
}

pid_t  CgiConnection::CreateCgiConnection(ClientConnection& client) {
  int pipe_fd[2];
  if (pipe(pipe_fd) == -1) {
    logError("pipe()");
    return -1;
  }
  pid_t pid = fork();
  if (pid == -1) {
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    logError("fork()");
    return -1;
  }
  if (pid != 0) {
    pollfd cgi_poll = {0, 0, 0};
    cgi_poll.fd = pipe_fd[WRITE];
    fcntl(cgi_poll.fd, F_SETFL, O_NONBLOCK);
    cgi_poll.events = POLLOUT;
    WebServ& webserv = client.getWebServ();
    webserv.AddNewConnection(
        cgi_poll, std::make_unique<CgiConnection>(pipe_fd, client, pid));
  }
  else
    StartCgiProcess(pipe_fd, client);
  return pid;
}


void  CgiConnection::StartCgiProcess(int pipe_fd[2], ClientConnection& client) {
  close(pipe_fd[0]);
  if (dup2(pipe_fd[1], STDOUT_FILENO) == -1) {
    close(pipe_fd[1]);
    std::_Exit(EXIT_FAILURE);
  }
  close(pipe_fd[1]);
  std::vector<std::string> env = client.PrepareCgiEvniron();
    //call execve
    std::_Exit(EXIT_FAILURE);
}
  
int CgiConnection::ReceiveData(pollfd& poll) {
  char  buffer[MAXBYTES];
  int   bytes_in;

  bytes_in = read(fd_, buffer, MAXBYTES);
  if (bytes_in == -1) {
    //error - stop reading
  }
  (void)client_;
  (void)poll;
  return 0;
}

int CgiConnection::SendData(pollfd& poll) {
  char  buffer[MAXBYTES];

  if (!file_.is_open()) {
    SwitchToRecieve();
    return 0;
  }
  file_.read(buffer, MAXBYTES);
  if (file_.fail()) {
    logError("CGI: couldn't read from fstream");
    return 1;
  }
  if (write(poll.fd, buffer, file_.gcount()) == -1) {
    logError("CGI: couldn't write date to child process");
    return 1;
  }
  if (file_.eof())
    SwitchToRecieve();
  return 0;
}

void  CgiConnection::SwitchToRecieve() {
  pollfd cgi_poll = {0, 0, 0};
  cgi_poll.fd = pipe_fd_[READ];
  fcntl(cgi_poll.fd, F_SETFL, O_NONBLOCK);
  cgi_poll.events = POLLIN;
  webserv_.SwitchCgiToReceive(pipe_fd_[WRITE], pipe_fd_[READ]);
}