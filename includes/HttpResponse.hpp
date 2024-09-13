/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 15:51:16 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/13 17:34:05 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <unordered_map>
#include <fstream>

class HttpResponse
{
    private:
        int         errorCode;
        std::unordered_map<std::string, std::string> contTypeMap;
        std::string contType;
        std::ifstream file;

        void initContMap(void);

    public:

        HttpResponse();
        ~HttpResponse();
        
        void assignContType(std::string resourcePath);
        std::string getContType() const;
        void setErrorCode(int errorCode);
};