/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 15:51:16 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/20 22:35:22 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP_
#define HTTPRESPONSE_HPP_

#include <iostream>
#include <map>
#include <fstream>
#include <sstream>
#include <poll.h>
#include "Location.hpp"

class ClientConnection;

class HttpResponse {

  private:
     using StringMap = std::map<std::string, std::string>;

  public:
    HttpResponse(std::string& status);
    HttpResponse(const HttpResponse& other)             = delete;
    HttpResponse& operator=(const HttpResponse& other)  = delete;
    ~HttpResponse() = default;

    void  SendResponse(ClientConnection& fd_info, pollfd &poll);
    void  ResetResponse(); 

  private:
    void  AssignContType(std::string resourcePath);
    void  ComposeHeader();
    int   CheckRedirections(ClientConnection& fd_info, Location& loc);
    void  LookupStatusMessage();
    
    int   SendHeader(int client_socket, std::string request_target) ;
    int   SendOneChunk(int client_socket, std::fstream &file);
    int   SendToClient(const int clientSocket, const char *msg, int length);

    const std::map<std::string, std::string>& getContTypeMap();
    const std::map<std::string, std::string>& getStatusMap();
    

    std::string                            header_;
    std::string&                           status_;
    // StringMap                              cont_type_map_;
    std::string                            cont_type_;
    std::string                            status_message_;
    // std::map<std::string, std::string>     status_map_;

    std::string                            additional_headers_;
};

#endif