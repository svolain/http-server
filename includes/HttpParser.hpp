/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vsavolai <vsavolai@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 12:16:12 by vsavolai          #+#    #+#             */
/*   Updated: 2024/10/02 14:43:55 by vsavolai         ###   ########.fr       */
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
# include <array>

#define MAXBYTES 8192

class HttpParser {
  public:
  
      HttpParser() = default;
      HttpParser(const std::string buffer);
      ~HttpParser() = default;
      
  
      std::string  get_method() const;
      std::string  get_resource_path() const;
      std::string  get_query_string() const;
      std::string  get_http_version() const;
      std::array<char, MAXBYTES>& get_request_body();
      int          get_error_code() const;
      bool         get_is_chunked() const;
      std::map<std::string, std::string>& get_headers();
      size_t       get_chunk_size() const;

      bool ParseRequest(const std::string buffer);
      bool CheckValidPath(std::string path);
      // void ClearMemory();


  private:
      int                        error_code_;
      size_t                     chunk_size_;
      std::string							   method_;
      std::string							   resource_path_;
      std::string							   query_string_;
      std::string							   http_version_;
      std::array<char, MAXBYTES> request_body_;
      std::map<std::string, 
        std::string>	            headers_ = {};
      bool                        is_chunked_;
};

#endif