/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 15:51:16 by klukiano          #+#    #+#             */
/*   Updated: 2024/10/21 16:49:41 by klukiano         ###   ########.fr       */
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
class HttpParser;

class HttpResponse {

  private:
     using StringMap = std::map<std::string, std::string>;

  public:
    HttpResponse(std::string& status);
    HttpResponse(const HttpResponse& other)             = delete;
    HttpResponse& operator=(const HttpResponse& other)  = delete;
    ~HttpResponse() = default;

    int   SendResponse(ClientConnection& fd_info, pollfd &poll);
    void  ResetResponse(); 

  private:
    void  AssignContType(std::string resourcePath);
    void  ComposeHeader(std::string location_header);
    // int   CheckRedirections(ClientConnection& fd_info, Location& loc);
    void  LookupStatusMessage();
    
    int   SendHeader(int client_socket, HttpParser& parser);
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

    // std::string                            location_header_;
};

#endif