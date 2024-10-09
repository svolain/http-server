/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vsavolai <vsavolai@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 12:16:12 by vsavolai          #+#    #+#             */
/*   Updated: 2024/10/09 15:19:50 by vsavolai         ###   ########.fr       */
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
# include <vector>
# include <array>

#define MAXBYTES 8192

class HttpParser {
 public:
  HttpParser() = default;
  HttpParser(const std::string buffer);
  ~HttpParser() = default;

  std::string                         getMethod() const;
  std::string                         getResourcePath() const;
  std::string                         getQueryString() const;
  std::string                         getHttpVersion() const;
  std::vector<char>&                  getRequestBody();
  int                                 getErrorCode() const;
  void                                setErrorCode(int error) ;
  bool                                getIsChunked() const;
  std::map<std::string, std::string>& getHeaders();
  size_t                              getChunkSize() const;
  void                                appendBody(std::vector<char> buffer, int bytesIn);

  const std::string&                  getHost() const;

  bool ParseHeader(const std::string& buffer);
  bool CheckValidPath(std::string path);
  // void ClearMemory();


 private:
  int                                 error_code_ = 0;
  size_t                              chunk_size_;
  std::string                         method_;
  std::string                         resource_path_;
  std::string                         query_string_;
  std::string                         http_version_;
  std::vector<char>                   request_body_;
  std::map<std::string, std::string>  headers_ = {};
  bool                                is_chunked_ = false;
};

#endif