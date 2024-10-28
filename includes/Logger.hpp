/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.hpppp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/01 14:01:10 by dshatilo          #+#    #+#             */
/*   Updated: 2024/10/21 11:30:56 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <ctime>
#include <iostream>
#include <sstream>
#include <string>

inline std::string currentDateTime() {
  std::time_t now = std::time(nullptr);
  char buf[sizeof "2024-01-01T23:59:59"];
  std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
  return buf;
}

template<typename... Args>
void logInfo(const Args&... args) {
  std::cout << "[" << currentDateTime() << "] [INFO] ";
  std::ostringstream oss;
  (oss << ... << args);
  std::cout << oss.str() << std::endl;
}

template<typename... Args>
void logDebug(const Args&... args) {
#ifdef DEBUG
    std::cout << "\033[94m[" << currentDateTime() << "] [DEBUG] ";
    std::ostringstream oss;
    (oss << ... << args);
    std::cout << oss.str() << "\033[0m" << std::endl;
#else
    (void)sizeof...(args);
#endif
}

template<typename... Args>
void logError(const Args&... args) {
  std::cerr << "\033[31m[" << currentDateTime() << "] [ERROR] ";
  std::ostringstream oss;
  (oss << ... << args);
  std::cerr << oss.str() << "\033[0m" << std::endl;
}

#endif //LOGGER_H