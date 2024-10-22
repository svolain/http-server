/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiConnection.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  dshatilo < dshatilo@student.hive.fi >     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/18 14:07:41 by dshatilo          #+#    #+#             */
/*   Updated: 2024/10/22 22:24:04 by  dshatilo        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <unistd.h>
#include "CgiConnection.hpp"
#include "Logger.hpp"
#include <signal.h>

CgiConnection::CgiConnection(int fd, ClientConnection& client, pid_t child_pid)
    : Connection(fd, 10), client_(client), child_pid_(child_pid) {}

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
    close(pipe_fd[1]);
    pollfd cgi_poll = {0, 0, 0};
    cgi_poll.fd = pipe_fd[0];
    fcntl(cgi_poll.fd, F_SETFL, O_NONBLOCK);
    cgi_poll.events = POLLIN;
    WebServ& webserv = client.getWebServ();
    webserv.AddNewConnection(
        cgi_poll, std::make_unique<CgiConnection>(pipe_fd[0], client, pid));
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
  (void)poll;
  logError("TRYING TO WRITE SEND RESPONSE FROM CGI!!!");
  return 0;
}
