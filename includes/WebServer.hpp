/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/06 12:01:28 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/06 12:37:08 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./TcpListener.hpp"

class WebServer : public TcpListener
{

	protected:
		virtual void onClientConnected() override;
		virtual void onClientDisconected() override;

		virtual void onMessageRecieved(int clientSocket, const char *msg, int length) override;

	public:

		WebServer();
		WebServer(const char *m_ipAddress, const char *m_port);
		WebServer(const WebServer &other);
		~WebServer();
		WebServer&	operator=(const WebServer& other);

};

