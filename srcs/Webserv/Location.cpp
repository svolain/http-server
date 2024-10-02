/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/16 11:49:17 by dshatilo          #+#    #+#             */
/*   Updated: 2024/10/02 14:42:08 by dshatilo         ###   ########.fr       */
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
