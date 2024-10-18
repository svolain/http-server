/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGIConnection.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:38:49 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/18 14:19:16 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGICONNECTION_HPP
#define CGICONNECTION_HPP

#include "Connection.hpp"

#include <fstream>


class CGIConnection : public Connection {
 public:
  CGIConnection(int fd, pid_t child_pid, std::fstream& file);
  CGIConnection(const CGIConnection& other) = delete;
  ~CGIConnection() override                 = default;
  
  int             ReceiveData(pollfd& poll) override;
  int             SendData(pollfd& poll) override;
  void            CleanupConnection() override;

 private:
  pid_t         child_pid_;
  std::fstream& file_;

};

#endif //CGICONNECTION_HPP