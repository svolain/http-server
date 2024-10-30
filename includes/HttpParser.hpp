/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vsavolai <vsavolai@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 12:16:12 by vsavolai          #+#    #+#             */
/*   Updated: 2024/10/30 17:17:12 by vsavolai         ###   ########.fr       */
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
# include "VirtualHost.hpp"
# include <random>
# include <cstdio>

# define MAXBYTES 8192

class VirtualHost;

class HttpParser {
 private:
  using LocationMap = std::map<std::string, Location>;
  using LocationPair = std::pair<std::string, Location>;

 public:
  HttpParser(ClientConnection& client);
  HttpParser(const HttpParser& other)             = delete;
  HttpParser& operator=(const HttpParser& other)  = delete;

  ~HttpParser() = default;

  bool        ParseHeader(const std::string& buffer);
  bool        HandleRequest();
  bool        WriteBody(std::vector<char>& buffer, int bytesIn);
  bool        IsBodySizeValid();
  void        ResetParser();
  int         OpenFile(std::string& filename);
  std::string getHost();
  std::string getMethod() const;
  std::string getRequestTarget() const;
  std::string getFileList() const;

 private:
  friend ClientConnection;

  bool        ParseStartLine(std::istringstream& request_stream);
  bool        ParseHeaderFields(std::istringstream& request_stream);
  bool        CheckPostHeaders();
  void        ParseCookies(const std::string& cookie_header);
  std::string TrimWhitespace(const std::string& str);
  void        HandleCookies();
  bool        UnChunkBody(std::vector<char>& buf);
  void        AppendBody(std::vector<char> buffer, int bytesIn);
  bool        HandlePostRequest(std::vector<char> request_body);
  bool        HandleMultipartFormData(const std::vector<char> &body,
                                      const std::string &contentType);
  bool        ParseMultiPartData(std::vector<char> &bodyPart);
  bool        HandleDeleteRequest();
  void        GenerateFileListHtml();
  bool        CheckValidPath();
  void        CreateDirListing(std::string& directory);
  bool        HandleGet(bool autoIndex);
  std::string InjectFileListIntoHtml(const std::string& html_path);
  bool        checkFile(std::vector<char> request_body);

  ClientConnection&                   client_;
  size_t                              content_length_ = 0;
  std::string                         method_;
  std::string                         request_target_;
  std::string                         query_string_;
  std::string                         file_list_;
  std::string                         index_;
  std::string                         session_id_;
  std::vector<char>                   request_body_;
  std::map<std::string, std::string>  headers_;
  std::map<std::string, std::string>  session_store_;
  bool                                is_chunked_ = false;
  std::string                         content_type_;
  std::string                         uploads_;
};

#endif