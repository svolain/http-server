/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:38:49 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/24 17:59:21 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENTCONNECTION_HPP
#define CLIENTCONNECTION_HPP

#include "Connection.hpp"
#include "HttpParser.hpp"
#include "HttpResponse.hpp"
#include <fstream>

class Socket;
class VirtualHost;
class WebServ;
class CgiConnection;

class ClientConnection : public Connection {
 public:
  ClientConnection(int fd, Socket& sock, WebServ& webserv);
  ClientConnection(const ClientConnection& other)             = delete;
  ~ClientConnection() override                                = default;

  int                       ReceiveData(pollfd& poll) override;
  int                       SendData(pollfd& poll) override;
  void                      ResetClientConnection();
  std::vector<std::string>  PrepareCgiEvniron();

 private:
  friend HttpParser;
  friend HttpResponse;
  friend CgiConnection;

  enum class Stage { kHeader,
                     kBody,
                     kCgi,
                     kResponse };
                     
  Stage         stage_ = Stage::kHeader;
  std::string   status_ = "200";
  Socket&       sock_;
  WebServ&      webserv_;
  VirtualHost*  vhost_ = nullptr;
  HttpParser    parser_;
  HttpResponse  response_;
  std::fstream  file_;
};

#endif //CLIENTCONNECTION_HPP