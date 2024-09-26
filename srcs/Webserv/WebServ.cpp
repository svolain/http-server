/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/06 15:30:31 by dshatilo          #+#    #+#             */
/*   Updated: 2024/09/26 14:12:30 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"
#include "ConfigParser.hpp"

#define TODO 123

WebServ::WebServ(const char* conf) : conf_(conf != nullptr ? conf : DEFAULT_CONF) {}

int WebServ::init() {
  
  {
    ConfigParser parser(conf_.c_str());
    if (parser.parse_config(this->sockets_))
      return 1;
  }
  
  int i = 0;
  for (auto it = sockets_.begin(); it != sockets_.end(); it ++, i ++){
    if (sockets_[i].init_server(pollFDs_))
      return 2;
    std::cout << "init the server on socket " << sockets_[i].get_socket() << std::endl; 
  }
  return (0);
}


bool WebServ::is_fd_listening(int sock, short revents, std::vector<pollfd> &copyFDs)
{
  for (auto it = sockets_.begin(); it != sockets_.end(); it ++){
    if ((*it).get_listening().fd == sock && (revents & POLLIN)){
      pollfd newClient;
      newClient.fd = accept(sock, nullptr, nullptr);
      if (newClient.fd != -1){
        fcntl(newClient.fd, F_SETFL, O_NONBLOCK);
        newClient.events = POLLIN;
        copyFDs.push_back(newClient);
        ConnectInfo& newclient_info = connection_map[newClient.fd];
        newclient_info.init_info(newClient.fd, &(*it));
        std::cout << "Created a new value in the map with fd == " << newclient_info.get_fd() << std::endl;
        int yes = 1;
        if (setsockopt(newClient.fd, SOL_SOCKET, MSG_NOSIGNAL, &yes,
            sizeof(int)) == -1) {
          std::cerr << "is_fd_listening: setsockopt() error"  << std::endl; 
        }
      }
      else
        std::cerr << "Error on accept()\n"; 
      return true;
    }
  }
  return false;
}

#define TIMEOUT   1000
#define MAXBYTES  16000

void WebServ::poll_available_fds(void){
  
  std::vector<pollfd> copyFDs = pollFDs_;
  for (size_t i = 0; i < copyFDs.size(); i++){
    std::ostringstream  oss;
    //for readability
    int fd = copyFDs[i].fd;
    short revents = copyFDs[i].revents;
    ConnectInfo* fd_info = nullptr;
    HttpParser* parser = nullptr;
    if (connection_map.find(fd) != connection_map.end()){
      fd_info = &connection_map.at(fd);
      (*fd_info).set_copyFD_index(i);
      parser = (*fd_info).get_parser();
    }
    //is it an inbound connection?
    if (!fd_info && is_fd_listening(fd, revents, copyFDs)){
      continue;
    }
    else if (revents & POLLHUP){
      std::cout << "hang up" << fd << " with i = " << i  << std::endl;
      close_connection(fd, i, copyFDs);
      continue;
    }
    else if (revents & POLLNVAL){
      std::cout << "invalid fd " << fd << " with i = " << i  << std::endl;
      //try to close anyway
      close_connection(fd, i, copyFDs);
      continue;
    }
    else if (revents & POLLOUT){
      /* TODO: check the case wehn bytesIn < 0 and we try to send */
      std::cout << "ready for write" << std::endl;
      //Get the host: part from the parser
      std::map <std::string, std::string> headers = parser->get_headers();
      //for readability, find the host name among the Vhosts for this connection
      std::map<std::string, VirtualHost>  *v_hosts_ = &fd_info->get_socket()->v_hosts_;
      std::map<std::string, VirtualHost>::iterator vhosts_it = (*v_hosts_).find(headers["Host"]);
      //if we've found the name then direct it to the proper vhost
      if (vhosts_it != (*v_hosts_).end()){
        std::cout << "found " << vhosts_it->second.get_name() << std::endl;
        (*fd_info).set_vhost(&vhosts_it->second);
        vhosts_it->second.on_message_recieved(fd_info, copyFDs);
      }
      else {
        /* wrong hostname. Forward to default at index 0*/
        vhosts_it = (*v_hosts_).begin();
        (*fd_info).set_vhost(&vhosts_it->second);
        vhosts_it->second.on_message_recieved(fd_info, copyFDs);
      }
    }
    else if (revents & POLLIN){
      char                buf[MAXBYTES];
      int                 bytesIn;
      size_t              request_size = 0;
      while (1){
        //TODO: add a Timeout timer for the client connection
        memset(&buf, 0, MAXBYTES);
        bytesIn = recv(fd, buf, MAXBYTES, 0);
        if (bytesIn < 0){
          close_connection(fd, i, copyFDs);
          perror("recv -1:");
          break;
        }
        else if (bytesIn == 0){
            /* with the current break after on_message_recieved
          we can't get here */
          close_connection(fd, i, copyFDs);
          break;
        }
        else if (bytesIn == MAXBYTES){
          // TODO?: implement Header Too Long error?
          oss << buf;
          request_size += bytesIn;
        }
        else {
          oss << buf;
          request_size += bytesIn;
          
          std::cout << "the whole request is:\n" << oss.str() << std::endl;
          if (!parser->parseRequest(oss.str().c_str()))
            std::cout << "false on parseRequest returned" << std::endl;
          /* TODO: add find_host method to the parser, add body too long check */
          if (request_size > SIZE_MAX)
            /* TODO: add body too long check in the parser */
            ;
          copyFDs[i].events = POLLIN | POLLOUT;
          break;
        }
      }
    }
  }
  pollFDs_ = std::move(copyFDs);
}

void WebServ::run() {
  std::cout << "Servers are ready.\n";
  
  while (1){
    /* the socketsReady from poll() 
      is the number of file descriptors with events that occurred. */
    int socketsReady = poll(pollFDs_.data(), pollFDs_.size(), TIMEOUT);
    if (socketsReady == -1){
      perror("poll: ");
      continue;;
    }
    if (!socketsReady){
      /* TODO: individual timer for the timeout close */
      std::cout << "poll() is closing connections on timeout..." << std::endl;
      for (size_t i = 1; i < pollFDs_.size(); i ++){
        close(pollFDs_[i].fd);
        pollFDs_.erase(pollFDs_.begin() + i);
      }	
      continue;
    }
    poll_available_fds();
  }
  
  close_all_connections();
  std::cout << "--- Shutting down the server ---" << std::endl;
}

void WebServ::close_all_connections(void)
{
  for (size_t i = 0; i < pollFDs_.size(); i++)
    close(pollFDs_[i].fd);
}

void WebServ::close_connection(int sock, int i, std::vector<pollfd> &copyFDs)
{
  close(sock);
  copyFDs.erase(copyFDs.begin() + i);
  connection_map.erase(sock);
}