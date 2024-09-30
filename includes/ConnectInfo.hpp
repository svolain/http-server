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
#include <poll.h>
#include <fstream>

class Socket;
class VirtualHost;

class ConnectInfo
{
  public:
    ConnectInfo() = default;
    ~ConnectInfo() = default;
    // ConnectInfo(const ConnectInfo &other);
    // ConnectInfo&	operator=(const ConnectInfo& other);

    void            InitInfo(int fd, Socket *sock);

    void            set_vhost(VirtualHost *vhost);
    void            set_is_sending(bool boolean);
    HttpParser*     get_parser();
    Socket*         get_socket();
    int             get_fd();
    bool            get_is_sending();
    std::ifstream&  get_file();
    
  private:
    int                       fd_;
    Socket*                   sock_;
    VirtualHost*              vhost_;
    HttpParser                parser_;
    std::ifstream             file_;
    bool                      is_sending_chunks;
    // bool                      is_reading_body;
};


#endif