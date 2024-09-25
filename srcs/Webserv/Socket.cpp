/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 12:55:06 by dshatilo          #+#    #+#             */
/*   Updated: 2024/09/25 16:27:34 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Socket.hpp"


Socket::Socket(std::string& socket, VirtualHost& v) : v_hosts_({{v.get_name(), v}}) {
  size_t colon = socket.find(':');
  address_ = socket.substr(0, colon);
  port_ = socket.substr(colon + 1);

}

std::string Socket::get_socket() {
  return address_ + ":" + port_;
}

void  Socket::add_virtual_host(VirtualHost& v) {
  v_hosts_[v.get_name()] = v;
}


int Socket::init_server(std::vector<pollfd> &pollFDs)
{
  struct addrinfo hints{}, *servinfo;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  int status = getaddrinfo(address_.c_str(), port_.c_str(), &hints, &servinfo);
  if (status  != 0){
    std::cerr << gai_strerror(status) << std::endl;
    return 1;
  }

  if ((listening_.fd = socket(servinfo->ai_family, servinfo->ai_socktype,
      servinfo->ai_protocol)) == -1) {
    std::cerr << "server: socket() error"  << std::endl; 
    return 2;
  }
  /* SO_REUSEADDR for TCP to handle the case when the server shuts down
    and we can't bind to the same socket if there's data left
    port goes into a TIME_WAIT state otherwise*/
  int yes = 1;
  if (setsockopt(listening_.fd, SOL_SOCKET, SO_REUSEADDR, &yes,
      sizeof(int)) == -1) {
    std::cerr << "server: setsockopt() error"  << std::endl; 
    return 3;
  }

  if (bind(listening_.fd, servinfo->ai_addr, servinfo->ai_addrlen) == -1){
    close(listening_.fd);
    std::cerr << "server: bind() error"  << std::endl; 
    return 4;
  }
  freeaddrinfo(servinfo);
  
  #define BACKLOG 10
  /* TODO: DEFINED IN THE BACKLOG PARAMETER */
  if (listen(listening_.fd, BACKLOG) == -1){
    std::cerr << "server: listen() error"  << std::endl; 
    return 5;
  }
  listening_.events = POLLIN;
  pollFDs.push_back(listening_);
  return 0;
}


pollfd Socket::get_listening() const{
  return listening_;
}