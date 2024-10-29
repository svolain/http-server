/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiConnection.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/18 14:07:41 by dshatilo          #+#    #+#             */
/*   Updated: 2024/10/25 12:35:57 by dshatilo         ###   ########.fr       */
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
      file_(client.file_),
      child_pid_(child_pid) {
  pipe_fd_[READ] = pipe_fd[READ];
  pipe_fd_[WRITE] = pipe_fd[WRITE];
  }

CgiConnection::~CgiConnection() {
  if (pipe_fd_[WRITE] != -1)
    close(pipe_fd_[WRITE]);
  close(pipe_fd_[READ]);
  kill(child_pid_, SIGTERM);
  if (HasTimedOut()) {
    client_.status_ = "504";
    file_.close();
  }
    //                                          not ready
  client_.stage_ = ClientConnection::Stage::kResponse;
}

pid_t  CgiConnection::CreateCgiConnection(ClientConnection& client) {
  int pipe_fd[2];
  if (pipe(pipe_fd) == -1) {
    logError("CGI: failed to pipe()");
    return -1;
  }
  pid_t pid = fork();
  if (pid == -1) {
    close(pipe_fd[0]);
    close(pipe_fd[1]);
    logError("CGI: failed to fork()");
    return -1;
  }
  if (pid != 0) {
    pollfd cgi_poll = {0, 0, 0};
    cgi_poll.fd = pipe_fd[WRITE];
    fcntl(cgi_poll.fd, F_SETFL, O_NONBLOCK);
    cgi_poll.events = POLLOUT;
    client.webserv_.AddNewConnection(
        cgi_poll, std::make_unique<CgiConnection>(pipe_fd, client, pid));
  }
  else
    StartCgiProcess(pipe_fd, client);
  return pid;
}


void  CgiConnection::StartCgiProcess(int pipe_fd[2], ClientConnection& client) {
  if (dup2(pipe_fd[READ], STDIN_FILENO) == -1 ||
      dup2(pipe_fd[WRITE], STDOUT_FILENO) == -1) {
    close(pipe_fd[READ]);
    close(pipe_fd[WRITE]);
    std::_Exit(EXIT_FAILURE);
  }
  close(pipe_fd[READ]);
  close(pipe_fd[WRITE]);
  std::vector<std::string> env_vec = client.PrepareCgiEvniron();
  std::vector<const char*>env;
  env.reserve(env_vec.size() + 1);
  for (size_t i = 0; i < env_vec.size(); ++i) {
    env[i] = env_vec[i].data();
  }
  env[env_vec.size()] = nullptr;
  // execve();
  std::_Exit(EXIT_FAILURE);
}

int CgiConnection::ReceiveData(pollfd& poll) {
  char  buffer[MAXBYTES];
  int   bytes_in;

  bytes_in = read(poll.fd, buffer, MAXBYTES);
  if (bytes_in == -1) {
    logError("CGI: failed to read from child process.");
    return 1;
  }
  if (bytes_in == 0) {
    logInfo("CGI: finished reading from child.");
    return 1;
  }
  if (!header_parsed_) {
    header_parsed_ = ParseCgiResponseHeaderFields(buffer);
    if (!header_parsed_) {
      client_.status_ = "502";
      return 1;
    }
  } else {
    file_.write(buffer, bytes_in);
  }
  return 0;
}

int CgiConnection::SendData(pollfd& poll) {
  char  buffer[MAXBYTES];

  if (!file_.is_open())
    return SwitchToRecieve();
  file_.read(buffer, MAXBYTES);
  if (file_.fail()) {
    logError("CGI: failed to read from fstream.");
    client_.status_ = "500";
    return 1;
  }
  if (write(poll.fd, buffer, file_.gcount()) == -1) {
    logError("CGI: failed to write data to child process.");
    client_.status_ = "500";
    return 1;
  }
  if (file_.eof())
    return SwitchToRecieve();
  return 0;
}

int CgiConnection::SwitchToRecieve() {
  client_.webserv_.SwitchCgiToReceive(pipe_fd_[WRITE], pipe_fd_[READ]);
  fd_ = pipe_fd_[READ];
  pipe_fd_[WRITE] = -1;

  file_.close();
  std::string filename = "/tmp/webserv/cgi_"  + std::to_string(fd_);
  file_.open(
      filename,
      std::fstream::in |
      std::fstream::out |
      std::fstream::app |
      std::ios::binary
  );
  if (!file_.is_open()) {
    logError("CGI: failed to open temporary response file for child process.");
    client_.status_ = "500";
    return 1;
  }
  std::remove(filename.c_str());
  return 0;
}

bool CgiConnection::ParseCgiResponseHeaderFields(char* buffer) {
  std::istringstream  response_stream(buffer);
  std::string line;
  while (std::getline(response_stream, line) && line != "\r") {
    size_t delim = line.find(":");
    if (delim == std::string::npos || line.back() != '\r') {
      logError("CGI: Wrong header line format.");
      return false;
    }
    std::string header = line.substr(0, delim + 1);
    std::string header_value = line.substr(delim + 1);
    headers_[header] = header_value;
  }

  if (!headers_.contains("Content-Type:")) {
    logError("CGI: failed to find \"Content-Type\" in headers.");
    return false;
  }

  if (headers_.contains("Status:")) {
    std::string status_line = headers_.at("Status:");
    headers_.erase("Status:");
    size_t delim = status_line.find(" ");
    if (delim != std::string::npos)
      status_line = status_line.substr(0, delim);
    client_.status_ = status_line;
  }

  if (line != "\r") {
    logError("CGI: Header Fields Too Large");
    client_.status_ = "502";
    return false;
  }

  while (std::getline(response_stream, line))
    file_ << line << "\n";

  return true;
}
