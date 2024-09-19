/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualHost.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 17:08:10 by  dshatilo         #+#    #+#             */
/*   Updated: 2024/09/19 16:19:20 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VIRTUALHOST_HPP_
#define VIRTUALHOST_HPP_

#include <iostream>
#include <string>
#include <map>
#include <utility>
#include "Location.hpp"



class VirtualHost {
 public:
  VirtualHost()                                    = default;
  VirtualHost(const VirtualHost& other)            = default;
  VirtualHost& operator=(const VirtualHost& other) = default;

  ~VirtualHost() = default;

  std::string get_name();
  void        set_name(std::string& name);
  void        set_size(std::string& size);
  void        set_error_page(std::string& code, std::string& path);
  void        set_location(std::string& path, Location& location);
  
 private:
  std::string                        name_;
  std::map<std::string, std::string> error_pages_;
  size_t                             client_max_body_size_ = 1048576;
  std::map<std::string, Location>    locations_;
  
};

#endif
