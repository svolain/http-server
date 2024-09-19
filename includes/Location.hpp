/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Location.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 17:09:33 by  dshatilo         #+#    #+#             */
/*   Updated: 2024/09/19 16:23:23 by klukiano         ###   ########.fr       */
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

  void set_allowed_methods(std::string& line);
  void set_redirection(std::string& code, std::string& path);
  void set_root(std::string& root);
  void set_auto_index(std::string& autoindex);
  void set_index(std::string& index);

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
