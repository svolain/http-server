/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConnectInfo.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/24 17:39:21 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/24 17:39:21 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConnectInfo.hpp"

void ConnectInfo::InitInfo(int fd, Socket *sock){
  fd_ = fd;
  sock_ = sock;
  is_sending_chunks = false;
}

void ConnectInfo::set_vhost(VirtualHost *vhost){
  vhost_ = vhost;
}

void ConnectInfo::set_is_sending(bool boolean){
  is_sending_chunks = boolean;
}

bool ConnectInfo::get_is_sending(){
  return is_sending_chunks;
}

HttpParser* ConnectInfo::get_parser(){
  return &parser_;
}

Socket* ConnectInfo::get_socket(){
  return sock_;
}

int ConnectInfo::get_fd(){
  return fd_;
}

std::ifstream& ConnectInfo::get_file(){
  return file_;
}