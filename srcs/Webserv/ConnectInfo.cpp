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

void ConnectInfo::init_info(int fd, Socket *sock){
  fd_ = fd;
  sock_ = sock;
  is_sending_chunks = false;
}

ConnectInfo::ConnectInfo(){
    std::cout << "called the constructor for Connect Info" << std::endl;
}

ConnectInfo::~ConnectInfo(){
    std::cout << "called the destructor for Connect Info" << std::endl;
}

ConnectInfo::ConnectInfo(const ConnectInfo &other){
  std::cout << "Connect Info copy const called" << std::endl;
  *this = other;
}

ConnectInfo& ConnectInfo::operator=(const ConnectInfo& other){
  if (this != &other)
  {
    fd_ = other.fd_;
    pollFD_index_ = other.pollFD_index_;
    sock_ = other.sock_;
    vhost_ = other.vhost_;
    parser_ = other.parser_;
    files_pos_ = other.files_pos_;
    is_sending_chunks = other.is_sending_chunks;
  }
  return *this;
}

void ConnectInfo::set_vhost(VirtualHost *vhost){
  vhost_ = vhost;
}

void ConnectInfo::set_copyFD_index(int i){
  pollFD_index_ = i;
}

void ConnectInfo::set_is_sending(bool boolean){
  is_sending_chunks = boolean;
}

int ConnectInfo::get_pollFD_index(){
  return pollFD_index_;
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

std::map <std::string, std::streampos>& ConnectInfo::get_file_map(){
  return files_pos_;
}