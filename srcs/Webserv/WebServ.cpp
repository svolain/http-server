
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

#define TODO 123

WebServ::WebServ(const char* conf)
    : conf_(conf != nullptr ? conf : DEFAULT_CONF) {
  if (conf == nullptr)
    logInfo("Configuration file not provided.\n\t\t\t     Using the "
            "default configuration file: /" + std::string(DEFAULT_CONF));
}

int WebServ::Init() {
  {
    ConfigParser parser(conf_);
    if (parser.ParseConfig(this->sockets_))
      return 1;
  }
  logDebug(ToString(), true);
  for (Socket& socket : sockets_) {
    if (socket.InitServer(pollFDs_))
      return 2;
    logDebug("init the server on socket " + socket.getSocket());
  }
  logInfo("Servers are ready");
  return 0;
}

#define TIMEOUT   5000

void WebServ::Run() {
  int socketsReady = 0;
  while (true) {
    /* the socketsReady from poll() 
      is the number of file descriptors with events that occurred. */
    socketsReady = poll(pollFDs_.data(), pollFDs_.size(), TIMEOUT);
    if (socketsReady == -1)
      perror("poll: ");
    else if (!socketsReady) {
      logInfo("poll() is closing connections on timeout...");
      for (auto it = connections_.begin(); it != connections_.end(); it++) //close all active connections
        it->second->CleanupConnection();
      connections_.clear();
      pollFDs_.resize(sockets_.size());
    } else
        PollAvailableFDs();
  }
  CloseAllConnections();
  logInfo("--- Shutting down the server ---");
}

void WebServ::PollAvailableFDs(void) {
  for (size_t i = 0; i < pollFDs_.size(); i++) {
    int fd = pollFDs_[i].fd;
    short revents = pollFDs_[i].revents;

    if (i < sockets_.size()) {
      CheckForNewConnection(fd, revents, i);
      continue;
    }
    Connection& connection = *connections_.at(fd);
    if (revents & POLLERR) {
      logDebug("error or read end has been closed", true);
      CloseConnection(connection, i);
    } else if (revents & POLLHUP) { 
      logDebug("Hang up: " + std::to_string(fd), true);
      CloseConnection(connection, i);
    } else if (revents & POLLNVAL) {
      logDebug("Invalid fd: " + std::to_string(fd));
      CloseConnection(connection, i);
    } else if (revents & POLLIN) {
      ReceiveData(connection, i);
    } else if (revents & POLLOUT)
      SendData(connection, pollFDs_[i]);
  }
}

void WebServ::CheckForNewConnection(int fd, short revents, int i) {
  if (revents & POLLIN) {
    pollfd new_client;
    new_client.fd = accept(fd, nullptr, nullptr);
    if (new_client.fd != -1) {
      /* O_NONBLOCK: No I/O operations on the file descriptor will cause the
            calling process to wait. */
      fcntl(new_client.fd, F_SETFL, O_NONBLOCK);
      new_client.events = POLLIN;
      pollFDs_.push_back(new_client);
      connections_.emplace(new_client.fd,
                           std::make_unique<ClientConnection>(new_client.fd,
                                                              sockets_[i]));
    }
  }
}

  /* find the host with the parser
    check if the permissions are good (Location)
    assign the vhost to the ConnecInfo class
    set bool to send body if all is good
    get back and try to read the body
    read for MAXBYTES and go back and continue next time
  */
void WebServ::ReceiveData(Connection& connection, size_t& i) {
  if (connection.ReceiveData(pollFDs_[i]))
    CloseConnection(connection, i);
}

void WebServ::SendData(Connection& fd_info, pollfd& poll) {
  fd_info.SendData(poll);
//   if (poll.events == POLLIN)
//     fd_info.ResetClientConnection();
}

void WebServ::CloseConnection(Connection& connection, size_t& i) {
  connection.CleanupConnection();
  connections_.erase(connection.fd_);
  pollFDs_.erase(pollFDs_.begin() + i);
  i--;
}

void WebServ::CloseAllConnections() {
  for (size_t i = 0; i < sockets_.size(); i++) //close all listening sockets
    close(pollFDs_[i].fd);
  for (auto it = connections_.begin(); it != connections_.end(); it++) //close all active connections
    it->second->CleanupConnection();
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
