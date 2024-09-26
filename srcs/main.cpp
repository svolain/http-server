/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/05 15:17:35 by shatilovdr        #+#    #+#             */
/*   Updated: 2024/09/26 14:02:50 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"


bool showResponse = 1;
bool showRequest = 1;

#include "WebServ.hpp"
#include <iostream>

int main(int ac, char **av){
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

