/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 11:49:17 by dshatilo          #+#    #+#             */
/*   Updated: 2024/09/19 16:23:23 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

void Location::set_allowed_methods(std::string& line) {
  if (line.find("GET") != std::string::npos)
    methods_[0] = true;
  if (line.find("HEAD") != std::string::npos)
    methods_[1] = true;
  if (line.find("POST") != std::string::npos)
    methods_[2] = true;
  if (line.find("DELETE") != std::string::npos)
    methods_[3] = true;
}

void Location::set_redirection(std::string& code, std::string& path) {
  redirection_.first = code;
  redirection_.second = path;
}

void Location::set_root(std::string& root) {
  root_ = root;
}

void Location::set_auto_index(std::string& autoindex) {
  autoindex_ = autoindex == "on" ? true : false;
}
void Location::set_index(std::string& index) {
  index_ = index;
}