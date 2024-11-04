/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 15:51:16 by klukiano          #+#    #+#             */
/*   Updated: 2024/11/04 18:45:35 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP_
#define HTTPRESPONSE_HPP_

#include <iostream>
#include <map>
#include <fstream>
#include <sstream>
#include <poll.h>
#include <vector>
#include "Location.hpp"

class ClientConnection;
class HttpParser;

class HttpResponse {

  private:
     using StringMap = std::map<std::string, std::string>;

  public:
    HttpResponse(ClientConnection& client);
    HttpResponse(const HttpResponse& other)             = delete;
    HttpResponse& operator=(const HttpResponse& other)  = delete;
    ~HttpResponse() = default;

    int   SendResponse(pollfd &poll);
    void  ResetResponse();

  private:
    void  AssignContType(std::string resourcePath);
    void  ComposeHeader();
    void  LookupStatusMessage();
    
    int   SendHeader(int client_socket, HttpParser& parser);
    int   SendOneChunk(int client_socket, std::fstream &file);
    int   SendToClient(const int clientSocket, const char *msg, int length);

    const std::map<std::string, std::string>& getContTypeMap();
    const std::map<std::string, std::string>& getStatusMap();
    

    ClientConnection& client_;
    std::string       header_;
    std::string       cont_type_;
    std::string       status_message_;
    std::vector<char> unsent_msg;
};

#endif