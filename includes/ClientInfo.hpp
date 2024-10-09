/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ClientInfo.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vsavolai <vsavolai@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:38:49 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/09 15:31:04 by vsavolai         ###   ########.fr       */
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
  // ClientInfo(int fd, Socket* sock);
  ClientInfo(const ClientInfo& other)             = delete;
  ClientInfo& operator=(const ClientInfo& other)  = delete;
  ~ClientInfo()                                   = default;

  void            InitInfo(int fd, Socket *sock);
  void            AssignVHost();

  int             RecvRequest(pollfd& poll);
  int         WriteBody(ClientInfo& fd_info, pollfd& poll);
  bool        UnChunkBody(std::vector<char>& buf);

  void            setVhost(VirtualHost *vhost);
  void            setIsSending(bool boolean);
  void            setIsParsingBody(bool boolean);
  void            setIsFileOpened(bool boolean);
  HttpParser&     getParser();
  Socket*         getSocket();
  VirtualHost*    getVhost();
  int             getFd();
  bool            getIsSending();
  std::ifstream&  getGetfile();
  std::ofstream&  getPostfile();

private:
  int            fd_;
  Socket*        sock_ = nullptr;
  VirtualHost*   vhost_ = nullptr;
  HttpParser     parser_;
  std::ofstream  postfile_;
  std::ifstream  getfile_;
  bool           is_sending_chunks_ = false;
  bool           is_parsing_body_ = false;
};

#endif