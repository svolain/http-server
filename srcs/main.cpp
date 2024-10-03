/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dshatilo <dshatilo@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/05 15:17:35 by shatilovdr        #+#    #+#             */
/*   Updated: 2024/10/03 16:31:06 by dshatilo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"


bool show_request = 1;
bool show_response = 0;

#include "WebServ.hpp"
#include <iostream>

int main(int ac, char **av){
  if (ac > 2) {
    std::cerr << "Usage: /webserv [configuration file]\n";
    return 1;
  }

  WebServ webServ(av[1]);
  if (webServ.Init())
    return (1);
  webServ.Run();
  return  0;
}

