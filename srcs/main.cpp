/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/05 15:17:35 by shatilovdr        #+#    #+#             */
/*   Updated: 2024/10/03 17:20:33 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

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

