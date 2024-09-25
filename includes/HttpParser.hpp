/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 12:16:12 by vsavolai          #+#    #+#             */
/*   Updated: 2024/09/25 13:35:53 by klukiano         ###   ########.fr       */
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

class HttpParser {
    public:
        HttpParser(void);
        HttpParser(const std::string buffer);
        ~HttpParser(void);
        
        bool parseRequest(const std::string buffer);
        
        std::string get_method() const;
        std::string get_resource_path() const;
        std::string get_query_string() const;
        std::string get_http_version() const;
        std::string get_request_body() const;
        int         get_error_code() const;
        std::map<std::string, std::string> get_headers() const;

        bool checkValidPath(std::string path);


    private:
        int                     error_code;
        std::string							method;
        std::string							resourcePath;
        std::string							queryString;
        std::string							httpVersion;
        std::string             requestBody;
		    std::map<std::string, 
          std::string>	        headers = {};
};

#endif