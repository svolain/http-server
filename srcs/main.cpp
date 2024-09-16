/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By:  dshatilo < dshatilo@student.hive.fi >     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/05 15:17:35 by shatilovdr        #+#    #+#             */
/*   Updated: 2024/09/16 09:12:02 by  dshatilo        ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"

int main(int argc, char* argv[]) {
  if (argc > 2) {
    std::cerr << "Usage: /webserv [configuration file]\n";
    return 1;
  }
  WebServ webserv(argv[1]);

  if (webserv.Init())
    return (1);
  webserv.Run();

}
