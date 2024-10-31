/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiConnection.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  dshatilo < dshatilo@student.hive.fi >     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:38:49 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/31 01:25:29 by  dshatilo        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGICONNECTION_HPP
#define CGICONNECTION_HPP

#include "Connection.hpp"
#include "ClientConnection.hpp"
#include "WebServ.hpp"

#include <fstream>

class CgiConnection : public Connection {
 public:
  CgiConnection(int read_fd,
                int write_fd,
                ClientConnection& client,
                pid_t child_pid);
  CgiConnection(const CgiConnection& other)             = delete;
  CgiConnection& operator=(const CgiConnection& other)  = delete;

  ~CgiConnection() override;

  static pid_t  CreateCgiConnection(ClientConnection& client);
  int           ReceiveData(pollfd& poll) override;
  int           SendData(pollfd& poll) override;

 private:
  static void StartCgiProcess(int read_fd, int write_fd,
                              ClientConnection& client);
  int         SwitchToReceive();
  bool        ParseCgiResponseHeaderFields(char* buffer);

  int                                 pipe_fd_[2];
  ClientConnection&                   client_;
  std::fstream&                       file_;
  pid_t                               child_pid_;
  bool                                header_parsed_ = false;
};

#endif //CGICONNECTION_HPP