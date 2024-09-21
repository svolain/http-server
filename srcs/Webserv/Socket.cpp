/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 12:55:06 by dshatilo          #+#    #+#             */
/*   Updated: 2024/09/21 17:46:38 by klukiano         ###   ########.fr       */
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

struct EventFlag {
    short flag;
    const char* description;
};

EventFlag eventFlags[] = {
            {POLLIN, "POLLIN (Data to read)"},
            {POLLOUT, "POLLOUT (Ready for writing)"},
            {POLLERR, "POLLERR (Error)"},
            {POLLHUP, "POLLHUP (Hang-up)"},
            {POLLNVAL, "POLLNVAL (Invalid FD)"},
            {POLLPRI, "POLLPRI (Urgent Data)"}
};

#define TIMEOUT		5000

#define MAXBYTES 16000

int Socket::poll_server(void)
{
  std::vector<pollfd> copyFDs = pollFDs_;
    /* the socketsReady from poll() 
      is the number of file descriptors with events that occurred. */
    int socketsReady = poll(copyFDs.data(), copyFDs.size(), TIMEOUT);
    if (socketsReady == -1){
      perror("poll: ");
      return 0;
    }
    if (!socketsReady){
      std::cout << "poll() is closing connections on timeout..." << std::endl;
      for (size_t i = 1; i < pollFDs_.size(); i ++)
      {
        close(pollFDs_[i].fd);
        pollFDs_.erase(pollFDs_.begin() + i);
      }	
      return 0;
    }
    for (size_t i = 0; i < copyFDs.size(); i++){
      //for readability
      int sock = copyFDs[i].fd;
      short revents = copyFDs[i].revents;
      //is it an inbound connection?
      if (sock == listening_.fd){
        if (revents & POLLIN){
        /* should we store the address of the connnecting client?
          for now just using nullptr */
        pollfd newClient;
        newClient.fd = accept(listening_.fd, nullptr, nullptr);
        if (newClient.fd != -1){
          newClient.events = POLLIN;
          pollFDs_.push_back(newClient);
          // std::cout << "Created a new connection" << std::endl;
        }
      }
        // onClientConnected();
    }
      else if (sock != listening_.fd && (revents & POLLHUP)){
        std::cout << "hang up" << sock << " with i = " << i  << std::endl;
        close(sock);
        pollFDs_.erase(pollFDs_.begin() + i);
        continue;
      }
      else if (sock != listening_.fd && (revents & POLLNVAL)){
        std::cout << "invalid fd " << sock << " with i = " << i  << std::endl;
        //try to close anyway
        close(sock);
        pollFDs_.erase(pollFDs_.begin() + i);
        continue;
      }
      //there is data to recv
      else if (sock != listening_.fd && (revents & POLLIN)){
        char                buf[MAXBYTES];
        int                 bytesIn;
        std::ostringstream  oss;
        size_t              body_count = 0;
        
        fcntl(sock, F_SETFL, O_NONBLOCK);
        /* do it in a loop until recv is 0? 
          would it be considered blocking?*/
        while (1){
          //TODO: add a Timeout timer for the client connection
          memset(&buf, 0, MAXBYTES);
          bytesIn = recv(sock, buf, MAXBYTES, 0);
          if (bytesIn < 0){
            close(sock);
            pollFDs_.erase(pollFDs_.begin() + i);
            perror("recv -1:");
            break ;
          }
          /* Clean parser memory, decide which virtual host it belongs to, choose
            from the list and pass it to the on_message_recived*/
          else if (bytesIn == 0){
             /* with the current break after on_message_recieved
            we can't get here */
            close(sock);
            pollFDs_.erase(pollFDs_.begin() + i);
            break;
          }
          else if (bytesIn == MAXBYTES){
            // TODO: implement error of body too long
            oss << buf;
            body_count += bytesIn;
          }
          else {
            oss << buf;
            body_count += bytesIn;

            if (!parser.parseRequest(oss.str().c_str()));
              std::cout << "false on parseRequest returned" << std::endl;
            std::cout << "\nrequestBody:\n" << parser.get_request_body() << std::endl;
            std::cout << "the err code is " << parser.get_error_code() << std::endl;

            /* TODO: add fin host method to the parser */
            auto it = v_hosts_.find("example.com");
            if (it != v_hosts_.end()){
              it->second.on_message_recieved(sock, parser);
            }
            else {
              /* TODO: wrong hostname. Forward to default? 
                What do we have at index 0?*/
              v_hosts_[0].on_message_recieved(sock, parser);
            }
           
            if (!recv(sock, buf, 0, 0))
              std::cout << "Client closed the connection" << std::endl;
            /* If the request is maxbytes we should not break the connection here */
            break ;
          }
        }
      }
      /* An unspecified event will trigget this loop 
        to see which flag is in the revents*/
      else {
        for (const auto& eventFlag : eventFlags) {
          if (revents & eventFlag.flag) 
            std::cout << eventFlag.description << std::endl;
        }
      }
      if (sock != listening_.fd && (revents & POLLOUT)){
        /* TODO: check the case wehn bytesIn < 0 and we try to send */
        std::cout << "ready for write" << std::endl;
      }
    }
    return 0;
}

int Socket::init_server()
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

  
  // for (addrinfo *addr = hints; addr != NULL; addr = addr->ai_next)
  //   {
  //      std::cout << "here" << std::endl;
  //   }

  if (bind(listening_.fd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
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
  /* POLLIN is read-ready, 
    POLLPRI is for out-of-band data, 
    is it related to building a packet from multiple packets?*/
  listening_.events = POLLIN | POLLPRI ;
  pollFDs_.push_back(listening_);
  return 0;
}

void Socket::close_all_connections(void)
{
  for (size_t i = 0; i < pollFDs_.size(); i++)
    close(pollFDs_[i].fd);
}
