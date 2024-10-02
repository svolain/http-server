/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 17:09:33 by  dshatilo         #+#    #+#             */
/*   Updated: 2024/10/02 14:43:05 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP_
#define LOCATION_HPP_

#include <utility>
#include <array>
#include <string>


class Location {
 private:
  using StringPair = std::pair<std::string, std::string>;

 public:
  Location(std::string& methods,
           StringPair& redirection,
           std::string& root,
           std::string& autoindex,
           std::string& index);
  Location() = default;
  Location(const Location& other)            = default;
  Location& operator=(const Location& other) = default;

  ~Location() = default;


 private:
  void SetAllowedMethods(std::string& line);

  std::array<bool, 4> methods_;
  StringPair          redirection_;
  std::string         root_;
  bool                autoindex_ = false;
  std::string         index_;
  //CGI
  //UPLOAD
};

#endif
