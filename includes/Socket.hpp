/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Socket.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/02 15:28:14 by dshatilo          #+#    #+#             */
/*   Updated: 2024/10/02 17:01:45 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SOCKET_HPP_
#define SOCKET_HPP_

#include "VirtualHost.hpp"
#include "HttpParser.hpp"

#include <unistd.h>
#include <poll.h>
#include <fcntl.h>
#include <vector>
// #include <string.h>


class Socket {
 private:
  using StringMap = std::map<std::string, std::string>;
  using LocationMap = std::map<std::string, Location>;

 public:
  Socket(std::string& listen,
         std::string& name,
         std::string& max_size,
         StringMap& errors,
         LocationMap& locations);
  Socket(const Socket& other)            = default;
  Socket& operator=(const Socket& other) = default;
  ~Socket() = default;

  std::string                         getSocket();
  pollfd                              get_listening() const;
  std::map<std::string, VirtualHost>& get_v_hosts();
  void                                AddVirtualHost(std::string& name,
                                                     std::string& max_size,
                                                     StringMap&   errors,
                                                     LocationMap& locations);
  int                                 InitServer(std::vector<pollfd> &pollFDs);

 private:
  std::string                         address_;
  std::string                         port_;
  pollfd                              listening_;
  std::map<std::string, VirtualHost>  v_hosts_;

};

#endif
