/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/05 15:17:35 by shatilovdr        #+#    #+#             */
/*   Updated: 2024/09/20 13:05:10 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"


bool showResponse = 0;
bool showRequest = 0;

// int main(int argc, char* argv[]) {
  // if (argc > 2) {
  //   std::cerr << "Usage: /webserv [configuration file]\n";
  //   return 1;
  // }
  // WebServ webserv(argv[1]);

  // if (webserv.Init())
  //   return (1);
  // webserv.Run();

#include "WebServ.hpp"
#include <iostream>

int main(int ac, char **av)
{
	// if (ac == 4)
	// {
	// 	try {
	// 		if (std::stoi(av[2]) == 1)
	// 			showRequest = 1;
	// 		if (std::stoi(av[3]) == 1)
	// 			showResponse = 1;
	// 	}
	// 	catch (const std::out_of_range &e){
	// 		std::cout << "num out of range" << std::endl;
	// 	}
	// 	std::cout << "showRequest: " << showRequest << ", showResponse: " << showResponse << std::endl;
	// }
	// else
	// 	std::cout << "no config, showResponse: " << showResponse << std::endl;
	// std::cout << "-------------" << std::endl;
		
	if (ac > 2) {
		std::cerr << "Usage: /webserv [configuration file]\n";
		return 1;
  }
	WebServ webServ(av[1]);
	
	if (webServ.init())
		return (1);
	std::cout << "Initialized a server" << std::endl;
	
	webServ.run();

	return  0;

}

