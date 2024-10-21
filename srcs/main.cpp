/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/05 15:17:35 by shatilovdr        #+#    #+#             */
/*   Updated: 2024/10/21 11:31:31 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include "Logger.hpp"
#include "WebServ.hpp"

int main(int ac, char** av) {
  if (ac > 2) {
    logError("Usage: /webserv [configuration file]");
    return 1;
  }

  WebServ webServ(av[1]);
  if (webServ.Init())
    return 1;
  webServ.Run();
  return  0;
}
