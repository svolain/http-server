/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectInfo.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:38:49 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/24 17:39:32 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONNECTINFO_HPP
#define CONNECTINFO_HPP

#include "HttpParser.hpp"
#include "Socket.hpp"
#include <poll.h>

class ConnectInfo
{
  public:
    ConnectInfo() = default
    //will add freeing
    ~ConnectInfo() = default;

  private:

    int                             fd;
    pollfd                          poll;
    Socket*                         sock;
    HttpParser                      parser;
    std::map
      <std::string, std::streampos> files_pos;
    
  
};


#endif