/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:38:49 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/19 18:24:55 by dshatilo         ###   ########.fr       */
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

  int             ReceiveData(pollfd& poll) override;
  int             SendData(pollfd& poll) override;
  void            ResetClientConnection();
  HttpParser&     getParser();
  VirtualHost*    getVhost();
  int             getFd();
  std::ifstream&  getGetfile();
  bool            getIsSending();
  void            setIsSending(bool boolean);

private:
  std::string    status_ = "200";
  Socket&        sock_;
  WebServ&       webserv_;
  VirtualHost*   vhost_ = nullptr;
  HttpParser     parser_;
  HttpResponse   response_;
  std::ifstream  getfile_;
  bool           is_sending_chunks_ = false;
  bool           is_parsing_body_ = false;
};

#endif //CLIENTCONNECTION_HPP