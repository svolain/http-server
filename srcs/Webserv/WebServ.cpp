
/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/06 15:30:31 by dshatilo          #+#    #+#             */
/*   Updated: 2024/09/30 17:16:35 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include "ConfigParser.hpp"
#include "WebServ.hpp"
#include "ClientConnection.hpp"
#include "CgiConnection.hpp"

#define TODO 123
bool run = true;


static void signalHandler(int signum) {
    std::cout << "\nSignal " << signum << " received" << std::endl;
    run = false;
}

WebServ::WebServ(const char* conf)
    : conf_(conf != nullptr ? conf : DEFAULT_CONF) {
  if (conf == nullptr)
    logInfo("Configuration file not provided.\n\t\t\t"
            "Using the default configuration file: /", DEFAULT_CONF);
}

int WebServ::Init() {
  {
    ConfigParser parser(conf_);
    if (parser.ParseConfig(this->sockets_))
      return 1;
  }
  logDebug(ToString());
  for (Socket& socket : sockets_) {
    if (socket.InitServer(pollFDs_))
      return 2;
    logDebug("init the server on socket ", socket.getSocket());
  }
  logInfo("Servers are ready");
  return 0;
}

#define TIMEOUT   5000

void WebServ::Run() {
  signal(SIGINT, signalHandler);

  int socketsReady = 0;
  while (run) {
    socketsReady = poll(pollFDs_.data(), pollFDs_.size(), TIMEOUT);
    if (socketsReady == -1)
      perror("poll: ");
    else if (!socketsReady) {
      logInfo("poll() is closing connections on timeout...");
      for (size_t i = sockets_.size(); i < pollFDs_.size(); ++i)
        close(pollFDs_[i].fd);
      connections_.clear();
      pollFDs_.resize(sockets_.size());
    } else
        PollAvailableFDs();
  }
  CloseAllConnections();
  logInfo("--- Shutting down the server ---");
}


void WebServ::AddNewConnection(pollfd& fd,
                               std::unique_ptr<Connection> connection) {
  pollFDs_.push_back(fd);
  connections_.emplace(fd.fd, std::move(connection));
}

void  WebServ::SwitchCgiToReceive(int olg_cgi_fd, int& new_cgi_fd) {
  pollfd cgi_poll = {new_cgi_fd, POLLIN, 0};
  fcntl(cgi_poll.fd, F_SETFL, O_NONBLOCK);
  AddNewConnection(cgi_poll, std::move(connections_.at(olg_cgi_fd)));

  connections_.erase(olg_cgi_fd);
  auto it = std::find_if(
      pollFDs_.begin(),
      pollFDs_.end(),
      [&](const pollfd& poll) {
        return poll.fd == olg_cgi_fd;
      }
  );
  pollFDs_.erase(it);
}

void WebServ::PollAvailableFDs(void) {
  for (int i = pollFDs_.size() - 1; i >= 0; --i) {
    int fd = pollFDs_[i].fd;
    short revents = pollFDs_[i].revents;

    if (i < static_cast<int>(sockets_.size())) {
      CheckForNewConnection(fd, revents, i);
      continue;
    }
    Connection& connection = *connections_.at(fd);
    if (revents & POLLERR) {
      logError("error or read end has been closed");
      CloseConnection(connection.fd_, i);
    } else if (revents & POLLHUP) { 
      logError("Hang up: ", fd);
      CloseConnection(connection.fd_, i);
    } 
    // else if (revents & POLLNVAL) {
    //   logError("Invalid fd: ", fd);
    //   CloseConnection(connection, i);} 
    else if (revents & POLLIN) {
      ReceiveData(connection, i);
    } else if (revents & POLLOUT)
      SendData(connection, i);
  }
}

void WebServ::CheckForNewConnection(int fd, short revents, int i) {
  if (revents & POLLIN) {
    pollfd new_client = {0, 0, 0};
    new_client.fd = accept(fd, nullptr, nullptr);
    if (new_client.fd != -1) {
      /* O_NONBLOCK: No I/O operations on the file descriptor will cause the
            calling process to wait. */
      fcntl(new_client.fd, F_SETFL, O_NONBLOCK);
      new_client.events = POLLIN;
      AddNewConnection(new_client,
                       std::make_unique<ClientConnection>(new_client.fd,
                                                          sockets_[i], *this));
    }
  }
}

void WebServ::ReceiveData(Connection& connection, int& i) {
  if (connection.ReceiveData(pollFDs_[i]))
    CloseConnection(connection.fd_, i);
}

void WebServ::SendData(Connection& connection, int& i) {
  if (connection.SendData(pollFDs_[i]))
    CloseConnection(connection.fd_, i);
}

void WebServ::CloseConnection(int fd, int& i) {
  close(fd);
  connections_.erase(fd);
  pollFDs_.erase(pollFDs_.begin() + i);
}

void WebServ::CloseAllConnections() {
  for (size_t i = 0; i < pollFDs_.size(); i++) //close all listening sockets and active connections
    close(pollFDs_[i].fd);
  //Destructor should handle map and vector erasing
}

std::string WebServ::ToString() const {
  std::string out("***Webserv configuration***\n");
  out += "Configuuration file used: " + conf_ + "\n";
  for (const auto& socket : sockets_) {
    out += "Server\n";
    out += socket.ToString() + "\n";
  }
  return out;
}
