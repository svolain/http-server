/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/03 14:48:15 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/06 16:22:52 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

int main()
{

	WebServer webServer("0.0.0.0", "8080");

	if (webServer.init())
		return (1);
	webServer.run();

	return  0;
}
