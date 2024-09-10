/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpListener.hpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/04 16:11:04 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/10 11:48:05 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>
#include <netdb.h> 
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>

#include <poll.h>
#include <sys/select.h>
#include <sys/epoll.h>

#include <vector>

#define PORT "8080"
#define TODO 123

class TcpListener
{
	private:
		const char* 		m_ipAddress;
		const char*			m_port;
		pollfd				listening;
		std::vector<pollfd>	m_pollFDs;

	protected:
		virtual void onClientConnected() = 0;
		virtual void onClientDisconected() = 0;

		virtual void onMessageRecieved(const int clientSocket, const char *msg, int length) = 0;

		int sendToClient(int clientSocket, const char *msg, int length);

		void broadcastToClients(int clientSocket, const char *msg, int length);

	public:

		TcpListener();
		TcpListener(const char *m_ipAddress, const char *m_port);
		~TcpListener();

		int	init();

		int run();

};


