/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpParser.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vsavolai <vsavolai@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/09 12:16:12 by vsavolai          #+#    #+#             */
/*   Updated: 2024/09/09 16:24:40 by vsavolai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPPARSER_HPP
# define HTTPPARSER_HPP

# include <iostream>
# include <map>
# include <vector>

class HttpParser {
    public:
        HttpParser(void);
        HttpParser(const std::string buffer);
        ~HttpParser(void);
        
        bool parseRequest(const std::string buffer);
        
        std::string getMethod() const;
        std::string getResourcePath() const;
        std::string getQueryString() const;
        std::string getHttpVersion() const;
        std::map<std::string, std::string> getHeaders() const;


    private:
        bool parseRequestLine(const std::string &line);
        bool parseHeaders(const std::vector<std::string> &headerLines);
        
        std::string							method;
		std::string							resourcePath;
		std::string							queryString;
		std::string							httpVersion;
		std::map<std::string, std::string>	headers = {};
};

#endif