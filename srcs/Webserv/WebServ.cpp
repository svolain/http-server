/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/06 15:30:31 by dshatilo          #+#    #+#             */
/*   Updated: 2024/09/23 17:53:09 by klukiano         ###   ########.fr       */
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

bool WebServ::is_sock_listening(int sock, short revents)
{
  for (auto it = sockets_.begin(); it != sockets_.end(); it ++){
    if ((*it).get_listening().fd == sock){
      if (revents & POLLIN){
      /* should we store the address of the connnecting client?
        for now just using nullptr */
        pollfd newClient;
        newClient.fd = accept(sock, nullptr, nullptr);
        if (newClient.fd != -1){
          Socket *connect_ptr = &(*it);
          newClient.events = POLLIN;
          pollFDs_.push_back(newClient);
          connection_map[newClient.fd] = connect_ptr;
        }
        else
          std::cerr << "Error on accept()\n"; 
        // onClientConnected();
      }
      return true;
    }
  }
  return false;
}

#define TIMEOUT   1000
#define MAXBYTES  16000

void WebServ::poll_available_fds(void){
  EventFlag eventFlags[] = {
    {POLLIN, "POLLIN (Data to read)"},
    {POLLOUT, "POLLOUT (Ready for writing)"},
    {POLLERR, "POLLERR (Error)"},
    {POLLHUP, "POLLHUP (Hang-up)"},
    {POLLNVAL, "POLLNVAL (Invalid FD)"},
    {POLLPRI, "POLLPRI (Urgent Data)"}
  };
  
  std::vector<pollfd> copyFDs = pollFDs_;
  for (size_t i = 0; i < copyFDs.size(); i++){
      //for readability
      int sock = copyFDs[i].fd;
      short revents = copyFDs[i].revents;
      //is it an inbound connection?
      if (is_sock_listening(sock, revents)){
        continue;
      }
      else if (revents & POLLHUP){
        std::cout << "hang up" << sock << " with i = " << i  << std::endl;
        close(sock);
        pollFDs_.erase(pollFDs_.begin() + i);
        connection_map.erase(sock);
        continue;
      }
      else if (revents & POLLNVAL){
        std::cout << "invalid fd " << sock << " with i = " << i  << std::endl;
        //try to close anyway
        close(sock);
        pollFDs_.erase(pollFDs_.begin() + i);
        connection_map.erase(sock);
        continue;
      }
      //there is data to recv
      else if (revents & POLLIN){
        char                buf[MAXBYTES];
        int                 bytesIn;
        std::ostringstream  oss;
        size_t              request_size = 0;

        memset(&buf, 0, MAXBYTES);
        fcntl(sock, F_SETFL, O_NONBLOCK);
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
            // TODO?: implement Header Too Long error?
            oss << buf;
            request_size += bytesIn;
          }
          else {
            oss << buf;
            request_size += bytesIn;

            HttpParser parser;
            std::cout << "the whole request is " << oss.str() << std::endl;
            if (!parser.parseRequest(oss.str().c_str()))
              std::cout << "false on parseRequest returned" << std::endl;
            std::cout << "\nrequestBody:\n" << parser.get_request_body() << std::endl;
            std::cout << "the err code is " << parser.get_error_code() << std::endl;
            /* TODO: add find_host method to the parser */

            std::map <std::string, std::string> headers = parser.get_headers();
            std::map<std::string, VirtualHost>  *v_hosts_ = &connection_map[sock]->v_hosts_;
            auto it = (*v_hosts_).find(headers["Host"]);
            if (it != (*v_hosts_).end()){
              std::cout << "found " << it->second.get_name() << std::endl;
              it->second.on_message_recieved(sock, parser);
            }
            else {
              /* TODO: wrong hostname. Forward to default? 
                What do we have at index 0?*/
              it = (*v_hosts_).begin();
              it->second.on_message_recieved(sock, parser);
            }
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
      if (revents & POLLOUT){
        /* TODO: check the case wehn bytesIn < 0 and we try to send */
        std::cout << "ready for write" << std::endl;
      }
    }
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
