
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

#include "WebServ.hpp"
#include "ConfigParser.hpp"

#define TODO 123

extern bool showResponse;
extern bool showRequest;

WebServ::WebServ(const char* conf) : conf_(conf != nullptr ? conf : DEFAULT_CONF) {}

int WebServ::init() {
  
  {
    ConfigParser parser(conf_.c_str());
    if (parser.ParseConfig(this->sockets_))
      return 1;
  }
  
  int i = 0;
  for (auto it = sockets_.begin(); it != sockets_.end(); it ++, i ++) {
    if (sockets_[i].InitServer(pollFDs_))
      return 2;
    std::cout << "init the server on socket " << sockets_[i].get_socket() << std::endl; 
  }
  return (0);
}

#define TIMEOUT   5000

void WebServ::run() {
  std::cout << "Servers are ready.\n";
  int socketsReady = 0;
  while (1) {
    /* the socketsReady from poll() 
      is the number of file descriptors with events that occurred. */
    socketsReady = poll(pollFDs_.data(), pollFDs_.size(), TIMEOUT);
    if (socketsReady == -1)
      perror("poll: ");
    else if (!socketsReady) {
      /* TODO: individual timer for the timeout close */
      std::cout << "poll() is closing connections on timeout..." << std::endl;
      for (size_t i = sockets_.size(); i < pollFDs_.size(); i ++) {
        close(pollFDs_[i].fd);
        pollFDs_.erase(pollFDs_.begin() + i);
      }	
      client_info_map_.clear();
    }
    else 
      PollAvailableFDs();
  }
  
  CloseAllConnections();
  std::cout << "--- Shutting down the server ---" << std::endl;
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
      client_info_map_[new_client.fd].InitInfo(new_client.fd, &(sockets_[i]));
    }
  }
}

#define MAXBYTES  8192


void WebServ::RecvFromClient(ConnectInfo* fd_info, size_t& i) {
  
  char                buf[MAXBYTES];
  int                 bytesIn;
  size_t              request_size = 0;
  std::ostringstream  oss;
  
  // std::vector<char>   oss;
  int fd = fd_info->get_fd();
  while (1) {
    //TODO: add a Timeout timer for the client connection
    memset(&buf, 0, MAXBYTES);
    bytesIn = recv(fd, buf, MAXBYTES, 0);
    if (bytesIn < 0) {
      CloseConnection(fd, i);
      perror("recv -1:");
      break;
    }
    else if (bytesIn == 0) {
      /* When a stream socket peer has performed an orderly shutdown, the
        return value will be 0 (the traditional "end-of-file" return) */
      CloseConnection(fd, i);
      break;
    }
    else if (bytesIn == MAXBYTES) {
      // TODO?: implement Header Too Long error?
      oss << buf;
      request_size += bytesIn;
    }
    else {
      oss << buf;
      request_size += bytesIn;
      if (showRequest)
        std::cout << "\nthe whole request is:\n" << oss.str() << std::endl;
      HttpParser* parser = fd_info->get_parser();
      // if (!parser->ParseRequest(oss.str().c_str()))
      if (!parser->ParseRequest(oss.str()))
        std::cout << "false on ParseRequest returned" << std::endl;
      if (request_size > SIZE_MAX)
        /* TODO: add body too long check in the parser */ ;
      if (fd_info->get_vhost() == nullptr)
        fd_info->AssignVHost();
      pollFDs_[i].events = POLLOUT;
      break;
    }

  /* find the host with the parser
    check if the permissions are good (Location)
    assign the vhost to the ConnecInfo class
    set bool to send body if all is good
    get back and try to read the body
    read for MAXBYTES and go back and continue next time
     */
  }
}

void WebServ::SendToClient(ConnectInfo* fd_info, pollfd& poll) {
  fd_info->get_vhost()->OnMessageRecieved(fd_info, poll);
}

void WebServ::PollAvailableFDs(void) {
  for (size_t i = 0; i < pollFDs_.size(); i++) {
    //for readability
    int fd = pollFDs_[i].fd;
    short revents = pollFDs_[i].revents;
    /* If fd is not a server fd */
    if (i < sockets_.size())
      CheckForNewConnection(fd, revents, i);
    else if (i >= sockets_.size() && client_info_map_.find(fd) != client_info_map_.end()) {
      ConnectInfo* fd_info = &client_info_map_.at(fd);
      if (!fd_info) {
        std::cerr << "error: couldnt find the fd in the client_info_map_" << std::endl;
      }
      else if (revents & POLLHUP) {
        std::cout << "hang up: " << fd << std::endl;
        CloseConnection(fd, i);
      }
      else if (revents & POLLNVAL) {
        std::cout << "invalid fd: " << fd << std::endl;
        CloseConnection(fd, i);
      }
      else if (revents & POLLIN)
        RecvFromClient(fd_info, i);
      else if (revents & POLLOUT)
        SendToClient(fd_info, pollFDs_[i]);
      }
  }
}

void WebServ::CloseAllConnections(void) {
  for (size_t i = 0; i < pollFDs_.size(); i++)
    close(pollFDs_[i].fd);
}

void WebServ::CloseConnection(int sock, size_t& i) {
  close(sock);
  pollFDs_.erase(pollFDs_.begin() + i);
  client_info_map_.erase(sock);
  i --;
}

