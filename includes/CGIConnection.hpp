/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIConnection.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:38:49 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/19 18:27:38 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGICONNECTION_HPP
#define CGICONNECTION_HPP

#include "Connection.hpp"
#include "WebServ.hpp"

#include <fstream>

class WebServ;

class CGIConnection : public Connection {
 public:
  CGIConnection(int fd, pid_t child_pid, std::fstream& file);
  CGIConnection(const CGIConnection& other) = delete;
  ~CGIConnection() override                 = default; // may be it is necessary to close pipe fd in the destructor

  static pid_t    CreateCGIConnection(WebServ& webserv_);
  int             ReceiveData(pollfd& poll) override;
  int             SendData(pollfd& poll) override;

 private:
  pid_t         child_pid_;
  std::fstream& file_;

};

#endif //CGICONNECTION_HPP