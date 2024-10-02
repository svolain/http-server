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

    void            InitInfo(int fd, Socket *sock);
    void            AssignVHost();

    void            set_vhost(VirtualHost *vhost);
    void            set_is_sending(bool boolean);
    void            set_is_parsing_body(bool boolean);
    HttpParser*     get_parser();
    Socket*         get_socket();
    VirtualHost*    get_vhost();
    int             get_fd();
    bool            get_is_sending();
    bool            get_is_parsing_body();
    std::ifstream&  get_file();
    
  private:
    int                       fd_;
    Socket*                   sock_;
    VirtualHost*              vhost_;
    HttpParser                parser_;
    std::ofstream             postfile_;
    std::ifstream             getfile_;
    bool                      is_sending_chunks_;
    bool                      is_parsing_body_;
};


#endif