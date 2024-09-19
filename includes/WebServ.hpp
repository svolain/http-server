/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/05 15:17:45 by shatilovdr        #+#    #+#             */
/*   Updated: 2024/09/19 16:22:29 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WebServ_HPP_
#define WebServ_HPP_

#include <netdb.h> 
#include <unistd.h>
#include <poll.h>
#include <deque>
#include "Socket.hpp"

#define DEFAULT_CONF "./conf/default.conf"

class WebServ {
 public:
  WebServ(char* config_file);
  WebServ(const WebServ& other)            = delete;
  WebServ& operator=(const WebServ& other) = delete;

  ~WebServ() = default;

  void  run();
  int   init();

  void send_chunked_response(int clientSocket, std::ifstream &file);
  void on_message_recieved(const int clientSocket, const char *msg, int length, short revents);

 private:

  std::string          conf_;
  std::deque<Socket>   sockets_;
  // std::vector<pollfd>  sockets_fd_;

  int send_to_client(const int clientSocket, const char *msg, int length);

  //older, del
  const char* 		ipAddress_;
  const char*			port_;
  pollfd				listening_;
  std::vector<pollfd>	pollFDs_;
};
#endif
