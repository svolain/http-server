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

class Socket;
class VirtualHost;

class ConnectInfo
{
  public:
    ConnectInfo() = default;
    //will add freeing
    ~ConnectInfo() = default;

    void        init_info(int fd, Socket *sock);

    void        set_vhost(VirtualHost *vhost);
    void        set_is_sending(bool boolean);
    void        set_copyFD_index(int i);
    HttpParser* get_parser();
    Socket*     get_socket();
    int         get_fd();
    int         get_pollFD_index();
    bool        get_is_sending();
    std::map
      <std::string, std::streampos>& get_file_map();


  private:

    int                       fd_;
    int                       pollFD_index_;
    Socket*                   sock_;
    VirtualHost*              vhost_;
    HttpParser                parser_;
    std::map
      <std::string, std::streampos> files_pos_;
    bool                      is_sending_chunks;
    
};


#endif