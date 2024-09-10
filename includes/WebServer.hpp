/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/06 12:01:28 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/10 17:51:32 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./TcpListener.hpp"

class WebServer : public TcpListener
{
	private:
		int  readRequest(std::string *content, std::vector<std::string> &parsed, int *errorCode);
		void  sendResponse(const int &clientSocket ,int errorCode, std::string *content, 
	  const std::string contType);

	protected:
		virtual void onClientConnected() override;
		virtual void onClientDisconected() override;

		virtual void onMessageRecieved(int clientSocket, const char *msg, int length) override;

	public:

		WebServer(const char *m_ipAddress, const char *m_port);

};

