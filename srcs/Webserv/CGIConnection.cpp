/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIConnection.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/18 14:07:41 by dshatilo          #+#    #+#             */
/*   Updated: 2024/10/21 11:31:31 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <fcntl.h>
#include <unistd.h>
#include "CGIConnection.hpp"
#include "Logger.hpp"

CGIConnection::CGIConnection(int fd, pid_t child_pid, std::fstream& file)
    : Connection(fd, 10), child_pid_(child_pid), file_(file) {}

pid_t  CGIConnection::CreateCGIConnection(WebServ& webserv_) {
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
    pollfd cgi_poll{0};
    cgi_poll.fd = pipe_fd[0];
    fcntl(cgi_poll.fd, F_SETFL, O_NONBLOCK);
    cgi_poll.events = POLLIN;
    webserv_.AddNewConnection(cgi_poll, std::make_unique<CGIConnection>());
    return pid;
  }
  else {
    //prepare env
    //call execve
    std::_Exit(EXIT_FAILURE);
  }
}

int CGIConnection::ReceiveData(pollfd& poll) {
  char  buffer[MAXBYTES];
  int   bytes_in;

  bytes_in = read(fd_, buffer, MAXBYTES);
  if (bytes_in == -1) {
    //error - stop reading
  }
  
}

int CGIConnection::SendData(pollfd& poll) {
  (void)poll;
}
