/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientConnection.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vsavolai <vsavolai@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:38:49 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/22 12:30:05 by vsavolai         ###   ########.fr       */
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
  bool            getIsSending();
  void            setIsSending(bool boolean);

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
  bool           is_sending_chunks_ = false;
};

#endif //CLIENTCONNECTION_HPP