/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/03 14:48:15 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/14 20:30:21 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"
#include <iostream>

bool showResponse = 0;
bool showRequest = 0;

int main(int ac, char **av)
{
	if (ac == 4)
	{
		try {
			if (std::stoi(av[2]) == 1)
				showRequest = 1;
			if (std::stoi(av[3]) == 1)
				showResponse = 1;
		}
		catch (const std::out_of_range &e){
			std::cout << "num out of range" << std::endl;
		}
		std::cout << "showRequest: " << showRequest << ", showResponse: " << showResponse << std::endl;
	}
	else
		std::cout << "no config, showResponse: " << showResponse << std::endl;
	std::cout << "-------------" << std::endl;
		
	WebServer webServer("0.0.0.0", "8080");
	if (webServer.init())
		return (1);
	std::cout << "Initialized a server" << std::endl;
	
	webServer.run();

	return  0;
}
