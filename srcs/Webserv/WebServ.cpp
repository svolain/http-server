
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

#define TIMEOUT 500

void WebServ::Run() {
  signal(SIGINT, signalHandler);

  int socketsReady = 0;
  while (run) {
    socketsReady = poll(pollFDs_.data(), pollFDs_.size(), TIMEOUT);
    if (socketsReady == -1) {
      logError("poll() returned -1.");
      break;
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

void  WebServ::SwitchCgiToReceive(int olg_cgi_fd, int new_cgi_fd) {
  pollfd cgi_poll = {new_cgi_fd, POLLIN, 0};
  // fcntl(cgi_poll.fd, F_SETFL, O_NONBLOCK);
  connections_.emplace(cgi_poll.fd, std::move(connections_.at(olg_cgi_fd)));

  close(olg_cgi_fd);
  connections_.erase(olg_cgi_fd);

  auto it = std::find_if(
      pollFDs_.begin(),
      pollFDs_.end(),
      [&](const pollfd& poll) {
        return poll.fd == olg_cgi_fd;
      }
  );
  *it = cgi_poll;
}

void  WebServ::SwitchClientToSend(int fd) {
  auto it = std::find_if(
      pollFDs_.begin(),
      pollFDs_.end(),
      [&](const pollfd& poll) {
        return poll.fd == fd;
      }
  );
  it->events = POLLOUT;
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
      logInfo("Connection ", fd,
              ": Error or closed read end detected, closing connection.");
      CloseConnection(connection.fd_, i);
    } // else if (revents & POLLHUP) {
    //   logInfo("Connection ", fd, ": Hang-up detected, closing connection.");
    //   CloseConnection(connection.fd_, i);
    // }
    // else if (revents & POLLNVAL) {
    //   logError("Invalid fd: ", fd);
    //   CloseConnection(connection, i);} 
    else if (revents & POLLIN || revents & POLLHUP) {
      ReceiveData(connection, i);
    } else if (revents & POLLOUT)
      SendData(connection, i);
    else if (connection.HasTimedOut()) {
      logInfo("Connection ", fd, ": Timeout detected, closing connection.");
      CloseConnection(connection.fd_, i);
    }
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

void WebServ::ReceiveData(Connection& connection, const int& i) {
  if (connection.ReceiveData(pollFDs_[i]))
    CloseConnection(pollFDs_[i].fd, i);
}


void WebServ::SendData(Connection& connection, const int& i) {
  if (connection.SendData(pollFDs_[i]))
    CloseConnection(pollFDs_[i].fd, i);
}

void WebServ::CloseConnection(int fd, const int& i) {
  connections_.erase(fd);
  pollFDs_.erase(pollFDs_.begin() + i);
}

void WebServ::CloseAllConnections() {
  for (size_t i = pollFDs_.size() - 1; i >= sockets_.size(); --i) {
    CloseConnection(pollFDs_[i].fd, static_cast<int>(i));
  }
  for (size_t i = 0; i < sockets_.size(); i++)
    close(pollFDs_[i].fd);
}

std::string WebServ::ToString() const {
  std::string out("***Webserv configuration***\n");
  out += "Configuration file used: " + conf_ + "\n";
  for (const auto& socket : sockets_) {
    out += "Server\n";
    out += socket.ToString() + "\n";
  }
  return out;
}
