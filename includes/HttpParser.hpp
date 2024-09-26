/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 12:16:12 by vsavolai          #+#    #+#             */
/*   Updated: 2024/09/26 13:21:20 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPARSER_HPP_
# define HTTPPARSER_HPP_

# include <iostream>
# include <map>
# include <sstream>
# include <string>
# include <algorithm>
# include <filesystem>
# include <unistd.h>
# include <fcntl.h>

class HttpParser {
  public:
      HttpParser(void);
      HttpParser(const std::string buffer);
      HttpParser(const HttpParser& other);
      HttpParser& operator=(const HttpParser& other);
      ~HttpParser(void);
      
      bool parseRequest(const std::string buffer);
      
      std::string get_method() const;
      std::string get_resource_path() const;
      std::string get_query_string() const;
      std::string get_http_version() const;
      std::string get_request_body() const;
      int         get_error_code() const;
      std::map<std::string, std::string> get_headers() const;

      bool checkValidPath(std::string path);


  private:
      int                     error_code_;
      std::string							method_;
      std::string							resource_path_;
      std::string							query_string_;
      std::string							http_version_;
      std::string             request_body_;
      std::map<std::string, 
        std::string>	        headers_ = {};
};

#endif