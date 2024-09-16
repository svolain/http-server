/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualHost.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 17:35:52 by  dshatilo         #+#    #+#             */
/*   Updated: 2024/09/16 13:55:30 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VirtualHost.hpp"

std::string VirtualHost::GetName() {
  return name_;
}

void VirtualHost::SetName(std::string& name) {
  name_ = name;
}

void VirtualHost::SetSize(std::string& size) {
  client_max_body_size_ = std::stoi(size);
  client_max_body_size_ *= (size.back() == 'M' ? 1048576 : 1024);
}

void VirtualHost::SetErrorPage(std::string& code, std::string& path) {
  error_pages_[code] = path;
}

void VirtualHost::SetLocation(std::string& path, Location& location) {
  locations_[path] = location;
}