/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 11:49:17 by dshatilo          #+#    #+#             */
/*   Updated: 2024/09/16 12:37:08 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

void Location::SetAllowedMethods(std::string& line) {
  if (line.find("GET") != std::string::npos)
    methods_[0] = true;
  if (line.find("HEAD") != std::string::npos)
    methods_[1] = true;
  if (line.find("POST") != std::string::npos)
    methods_[2] = true;
  if (line.find("DELETE") != std::string::npos)
    methods_[3] = true;
}

void Location::SetRedirection(std::string& code, std::string& path) {
  redirection_.first = code;
  redirection_.second = path;
}

void Location::SetRoot(std::string& root) {
  root_ = root;
}

void Location::SetAutoindex(std::string& autoindex) {
  autoindex_ = autoindex == "on" ? true : false;
}
void Location::SetIndex(std::string& index) {
  index_ = index;
}