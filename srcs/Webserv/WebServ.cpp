
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
      client_info_map[new_client.fd].InitInfo(new_client.fd, &(sockets_[i]));
    }
  }
}

#define TIMEOUT   15000
#define MAXBYTES  8192

void WebServ::RecvFromClient(ConnectInfo* fd_info, size_t& i) {
  
  char                buf[MAXBYTES];
  int                 bytesIn;
  size_t              request_size = 0;
  std::ostringstream  oss;
  
  // ReadHeader()
  // VirtualHost::ReadBody
  int fd = (*fd_info).get_fd();
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
    else  {
      oss << buf;
      request_size += bytesIn;
      
      std::cout << "the whole request is:\n" << oss.str() << std::endl;
      HttpParser* parser = (*fd_info).get_parser();
      if (!parser->ParseRequest(oss.str().c_str()))
        std::cout << "false on ParseRequest returned" << std::endl;
      /* TODO: add find_host method to the parser, add body too long check */
      if (request_size > SIZE_MAX)
        /* TODO: add body too long check in the parser */
        ;
      pollFDs_[i].events = POLLIN | POLLOUT;
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

void WebServ::SendToClient(ConnectInfo* fd_info, size_t& i) {

  HttpParser* parser = (*fd_info).get_parser();
  std::cout << "ready for write" << std::endl;
  //Get the host: part from the parser
  std::map <std::string, std::string> headers = parser->get_headers();
  //for readability, find the host name among the Vhosts for this connection
  std::map<std::string, VirtualHost>  *v_hosts_ = &fd_info->get_socket()->get_v_hosts();
  std::map<std::string, VirtualHost>::iterator vhosts_it = (*v_hosts_).find(headers["Host"]);
  //if we've found the name then direct it to the proper vhost
  if (vhosts_it != (*v_hosts_).end()){
    std::cout << "found " << vhosts_it->second.get_name() << std::endl;
    (*fd_info).set_vhost(&vhosts_it->second);
    vhosts_it->second.OnMessageRecieved(fd_info, pollFDs_[i]);
  }
  else {
    /* wrong hostname. Forward to default at index 0*/
    vhosts_it = (*v_hosts_).begin();
    (*fd_info).set_vhost(&vhosts_it->second);
    vhosts_it->second.OnMessageRecieved(fd_info, pollFDs_[i]);
  }
}

void WebServ::PollAvailableFDs(void) {
  for (size_t i = 0; i < pollFDs_.size(); i++) {
    //for readability
    int fd = pollFDs_[i].fd;
    short revents = pollFDs_[i].revents;
    ConnectInfo* fd_info = nullptr;
    if (i >= sockets_.size() && client_info_map.find(fd) != client_info_map.end())
      fd_info = &client_info_map.at(fd);
    else if (i >= sockets_.size() && client_info_map.find(fd) == client_info_map.end())
    {
      std::cout << "we must not be here" << std::endl;
      exit (123);
    }
    /* If fd is not a server fd */
    if (i < sockets_.size())
      CheckForNewConnection(fd, revents, i);
    else if (revents & POLLHUP) {
      std::cout << "hang up: " << fd << std::endl;
      CloseConnection(fd, i);
    }
    else if (revents & POLLNVAL) {
      std::cout << "invalid fd: " << fd << std::endl;
      CloseConnection(fd, i);
    }
    else if (revents & POLLOUT)
      SendToClient(fd_info, i);
    else if (revents & POLLIN)
      RecvFromClient(fd_info, i);
  }
}

void WebServ::run()  {
  std::cout << "Servers are ready.\n";
  
  while (1) {
    /* the socketsReady from poll() 
      is the number of file descriptors with events that occurred. */
    int socketsReady = poll(pollFDs_.data(), pollFDs_.size(), TIMEOUT);
    if (socketsReady == -1) {
      perror("poll: ");
      continue;;
    }
    if (!socketsReady) {
      /* TODO: individual timer for the timeout close */
      std::cout << "poll() is closing connections on timeout..." << std::endl;
      for (size_t i = sockets_.size(); i < pollFDs_.size(); i ++) {
        close(pollFDs_[i].fd);
        pollFDs_.erase(pollFDs_.begin() + i);
      }	
      continue;
    }
    PollAvailableFDs();
  }
  
  CloseAllConnections();
  std::cout << "--- Shutting down the server ---" << std::endl;
}

void WebServ::CloseAllConnections(void) {
  for (size_t i = 0; i < pollFDs_.size(); i++)
    close(pollFDs_[i].fd);
}

void WebServ::CloseConnection(int sock, size_t& i) {
  close(sock);
  pollFDs_.erase(pollFDs_.begin() + i);
  client_info_map.erase(sock);
  i --;
}

