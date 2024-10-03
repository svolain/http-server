/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 11:49:17 by dshatilo          #+#    #+#             */
/*   Updated: 2024/10/03 13:03:16 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Location.hpp"

Location::Location(std::string& methods,
                   StringPair&  redirection,
                   std::string& root,
                   std::string& autoindex,
                   std::string& index)
    : methods_{false},
      redirection_(redirection),
      root_(root),
      autoindex_(autoindex == "on" ? true : false),
      index_(index) {
  SetAllowedMethods(methods);
}

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

std::string Location::ToString() const {
  const char* methodNames[] = {"GET ", "HEAD ", "POST ", "DELETE"};

  std::string out;
  out += std::string(31, ' ') + "Allowed methods: ";
  for (int i = 0; i < 4; ++i) {
    if (methods_[i]) 
      out += methodNames[i];
  }
  out += "\n";
  if (!redirection_.first.empty()) {
    out += std::string(31, ' ') + "Redirection: ";
    out += redirection_.first + " " + redirection_.second + "\n";
  }
  if (!root_.empty()) {
    out += std::string(31, ' ') + "Root: " + root_ + "\n";
  }
  out += std::string(31, ' ') + "Autoindex: ";
  out += autoindex_ ? "on\n" : "off\n";
  if (!index_.empty()) {
    out += std::string(31, ' ') + "Index: " + index_;
  }
  return out;
}
