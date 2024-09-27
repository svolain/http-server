/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 15:51:16 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/27 17:15:05 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP_
#define HTTPRESPONSE_HPP_

#include <iostream>
#include <map>
#include <fstream>
#include <sstream>

class ConnectInfo;

class HttpResponse
{
    private:
        
        int                            error_code_;
        std::map
            <std::string, std::string> cont_type_map_;
        std::string                    cont_type_;
        std::string                    error_code_message_;
        std::string                    header_;

        void initContMap(void);
        void lookupErrMessage(void);

    public:

        HttpResponse();
        ~HttpResponse();
        
        void assign_cont_type_(std::string resourcePath);
     
        void            set_error_code_(int error_code_);
        void            open_file(std::string& resource_path, std::ifstream& file);
        void            compose_header(void);
        std::string     get_cont_type_() const;
        std::string     get_header_() const;
        std::string     get_error_codeMessage() const;
};

#endif