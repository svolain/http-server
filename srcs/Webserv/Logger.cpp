/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Logger.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/01 14:01:57 by dshatilo          #+#    #+#             */
/*   Updated: 2024/10/01 22:35:42 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Logger.h"
#include <iostream>
#include <ctime>

std::string currentDateTime() {
  std::time_t now = std::time(nullptr);
  char buf[sizeof "2021-12-31T23:59:59"];
  std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", std::localtime(&now));
  return buf;
}

void logInfo(const std::string& message) {
  std::cout << "[" << currentDateTime() << "] [INFO] " << message << '\n';
}

void logDebug(const std::string& message, bool flag) {
  if (flag)
    std::cout << "\033[94m[" << currentDateTime() << "] [DEBUG] " << message << "\n\033[0m";
}

void logError(const std::string& message) {
  std::cerr << "\033[31m[" << currentDateTime() << "] [ERROR] " << message << "\n\033[0m";
}
