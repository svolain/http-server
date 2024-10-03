/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  dshatilo < dshatilo@student.hive.fi >     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 17:09:33 by  dshatilo         #+#    #+#             */
/*   Updated: 2024/10/03 23:40:47 by  dshatilo        ###   ########.fr       */
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
  Location(const Location& other)            = default;
  Location& operator=(const Location& other) = delete;

  ~Location() = default;

  std::string ToString() const;

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
