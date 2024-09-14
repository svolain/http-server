/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/06 12:01:28 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/14 18:24:01 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "./TcpListener.hpp"

#define PRINTRESPONSE true

class WebServer : public TcpListener
{
	private:
		std::ifstream openFile(std::string resourcePath);
		void  composeHeader(const int &clientSocket, int errorCode, 
	const std::string contType);

	protected:
		virtual void onClientConnected() override;
		virtual void onClientDisconected() override;

		virtual void onMessageRecieved(int clientSocket, const char *msg, int length, short revents) override;

	public:

		WebServer(const char *m_ipAddress, const char *m_port);

};

