/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientInfo.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:38:49 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/04 15:42:31 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ClientInfo_HPP
#define ClientInfo_HPP

#include "HttpParser.hpp"
#include <poll.h>
#include <fstream>

class Socket;
class VirtualHost;

class ClientInfo
{
 public:
  ClientInfo() = default;
  ClientInfo(int fd, Socket* sock);
  ~ClientInfo() = default;

  void            InitInfo(int fd, Socket *sock);
  void            AssignVHost();

  void            setVhost(VirtualHost *vhost);
  void            setIsSending(bool boolean);
  void            setIsParsingBody(bool boolean);
  void            setIsFileOpened(bool boolean);
  HttpParser&     getParser();
  Socket*         getSocket();
  VirtualHost*    getVhost();
  int             getFd();
  bool            getIsSending();
  bool            getIsParsingBody();
  bool            getIsFileOpened();
  std::ifstream&  getGetfile();
  std::ofstream&  getPostfile();

private:
  int            fd_;
  Socket*        sock_;
  VirtualHost*   vhost_;
  HttpParser     parser_;
  std::ofstream  postfile_;
  std::ifstream  getfile_;
  bool           is_sending_chunks_;
  bool           is_parsing_body_;
  bool           is_file_opened_;
};

#endif