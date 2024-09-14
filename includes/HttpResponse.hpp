/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 15:51:16 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/14 20:33:31 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <unordered_map>
#include <fstream>
#include <sstream>

class HttpResponse
{
    private:
        int         errorCode;
        std::unordered_map<std::string, std::string> contTypeMap;
        std::string contType;
        std::ifstream file;
        std::string header;

        void initContMap(void);

    public:

        HttpResponse();
        ~HttpResponse();
        
        void assignContType(std::string resourcePath);
     
        void setErrorCode(int errorCode);
        void openFile(std::string resourcePath);
        void composeHeader(void);
        std::ifstream& getFile(void);
        std::string getContType() const;
        std::string getHeader() const;
};