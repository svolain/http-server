/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:38:49 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/23 08:58:20 by dshatilo         ###   ########.fr       */
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

class ClientConnection : public Connection {
 public:
  ClientConnection(int fd, Socket& sock, WebServ& webserv);
  ClientConnection(const ClientConnection& other)             = delete;
  ~ClientConnection() override                                = default;

  int                       ReceiveData(pollfd& poll) override;
  int                       SendData(pollfd& poll) override;
  void                      ResetClientConnection();
  std::vector<std::string>  PrepareCgiEvniron();
  WebServ&                  getWebServ();
  std::fstream&             getFile();
  void                      setStatus(const std::string& status);


private:
  friend HttpParser;
  friend HttpResponse;

  enum class Stage { kHeader,
                     kBody,
                     kCgi,
                     kResponse };
                     
  Stage          stage_ = Stage::kHeader;
  std::string    status_ = "200";
  Socket&        sock_;
  WebServ&       webserv_;
  VirtualHost*   vhost_ = nullptr;
  HttpParser     parser_;
  HttpResponse   response_;
  std::fstream   file_;
};

#endif //CLIENTCONNECTION_HPP