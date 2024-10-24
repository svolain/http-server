/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CgiConnection.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:38:49 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/24 17:30:46 by dshatilo         ###   ########.fr       */
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
  CgiConnection(int pipe_fd[2], ClientConnection& client, pid_t child_pid);
  CgiConnection(const CgiConnection& other)             = delete;
  CgiConnection& operator=(const CgiConnection& other)  = delete;

  ~CgiConnection() override;

  static pid_t  CreateCgiConnection(ClientConnection& client);
  static void   StartCgiProcess(int pipe_fd[2], ClientConnection& client);
  int           ReceiveData(pollfd& poll) override;
  int           SendData(pollfd& poll) override;
  int           SwitchToRecieve();

 private:
  int               pipe_fd_[2];
  ClientConnection& client_;
  std::fstream&     file_;
  pid_t             child_pid_;

};

#endif //CGICONNECTION_HPP