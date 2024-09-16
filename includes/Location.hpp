/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 17:09:33 by  dshatilo         #+#    #+#             */
/*   Updated: 2024/09/16 14:07:14 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOCATION_HPP_
#define LOCATION_HPP_

#include <utility>
#include <string>


class Location {
 public:
  Location()                                 = default;
  Location(const Location& other)            = default;
  Location& operator=(const Location& other) = default;

  ~Location() = default;

  void SetAllowedMethods(std::string& line);
  void SetRedirection(std::string& code, std::string& path);
  void SetRoot(std::string& root);
  void SetAutoindex(std::string& autoindex);
  void SetIndex(std::string& index);

 private:

  bool                          methods_[4] = {false};
  std::pair<std::string, std::string>   redirection_;
  std::string                   root_;
  bool                          autoindex_ = false;
  std::string                   index_;
  //CGI
  //UPLOAD
};

#endif
