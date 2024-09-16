/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/06 12:01:28 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/15 18:51:49 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "./TcpListener.hpp"

class WebServer : public TcpListener
{
	protected:
		virtual void onClientConnected() override;
		virtual void onClientDisconected() override;

		virtual void onMessageRecieved(int clientSocket, const char *msg, int length, short revents) override;

	public:

		WebServer(const char *m_ipAddress, const char *m_port);
		void sendChunkedResponse(int clientSocket, std::ifstream &file);
};

