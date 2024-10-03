/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  dshatilo < dshatilo@student.hive.fi >     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 15:51:16 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/03 23:53:29 by  dshatilo        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP_
#define HTTPRESPONSE_HPP_

#include <iostream>
#include <map>
#include <fstream>
#include <sstream>

class ClientInfo;

class HttpResponse
{
 public:
  HttpResponse();
  ~HttpResponse();

  void            AssignContType(std::string resourcePath);
  void            OpenFile(std::string& resource_path, std::ifstream& file);
  void            ComposeHeader(void);
  std::string     getContType() const;
  std::string     getHeader() const;
  std::string     getErrorCodeMessage() const;
  void            setErrorCode(int error_code_);

 private:
  int                                error_code_;
  std::map<std::string, std::string> cont_type_map_;
  std::string                        cont_type_;
  std::string                        error_code_message_;
  std::string                        header_;

  void InitContMap(void);
  void lookupErrMessage(void);
};

#endif