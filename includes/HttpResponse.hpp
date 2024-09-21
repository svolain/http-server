/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/13 15:51:16 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/21 17:34:00 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP_
#define HTTPRESPONSE_HPP_

#include <iostream>
#include <unordered_map>
#include <fstream>
#include <sstream>

class HttpResponse
{
    private:
        
        int                            error_code_;
        std::unordered_map
            <std::string, std::string> cont_type_map_;
        std::string                    cont_type_;
        std::string                    error_code_message_;
        // std::ifstream               file_;
        std::streampos                 saved_position_; 

        std::string                    header_;

        void initContMap(void);
        void lookupErrMessage(void);

    public:

        HttpResponse();
        HttpResponse(const HttpResponse &other) = default;
        ~HttpResponse();
        
        void assign_cont_type_(std::string resourcePath);
     
        void set_error_code_(int error_code_);
        void open_file(std::string resourcePath);
        void compose_header(void);
        std::ifstream& get_file_(void);
        std::string get_cont_type_() const;
        std::string get_header_() const;
        std::string get_error_codeMessage() const;
};

#endif