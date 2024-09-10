/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/06 12:01:28 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/10 12:09:46 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./TcpListener.hpp"

class WebServer : public TcpListener
{
	private:
		void sendText(std::string &content, std::vector<std::string> &parsed, int &errorCode);
		void sendResponse(int &errorCode, std::string &content, const int &clientSocket, 
			const char *contType);

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

