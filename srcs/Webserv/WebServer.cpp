
#include "WebServer.hpp"
#include <sstream>



WebServer::WebServer()
{
	std::cout << "Default Constructor for WebServer called" << std::endl;
}

WebServer::WebServer(const char *m_ipAddress, const char *m_port)
	: TcpListener(m_ipAddress, m_port)
{
	std::cout << "Paramterized Constructor for WebServer called" << std::endl;
}

WebServer::WebServer(const WebServer &other)
{
	std::cout << "Copy constuctor for WebServer called" << std::endl;
	*this = other;
}

WebServer::~WebServer()
{
	std::cout << "Destructor for WebServer called" << std::endl;
}

WebServer& WebServer::operator=(const WebServer& other)
{
	std::cout << "Assignment operator for WebServer called" << std::endl;
	if (this != &other)
	{
		return *this;
	}
	return *this;
}

void WebServer::onMessageRecieved(int clientSocket, const char *msg, int length)
{
	//GET /index.html HTTP/1.1

	//Parse out the document requested
	//Open the document ini the loacl files system
	//Write the document back to the client
	(void)length;
	(void)msg;
	std::ostringstream oss;
	oss << "HTTP/1.1 200 OK\r\n";
	oss << "Cache-Control: no-cache, private\r\n";
	oss << "Content-Type: text/plain\r\n";
	oss << "Content-Length: 5\r\n";
	oss << "\r\n";
	oss << "hello wwww";

	if(sendToClient(clientSocket, oss.str().c_str(), oss.str().size() + 1) < 0)
		exit (123);

}

void WebServer::onClientConnected()
{
	;
}

void WebServer::onClientDisconected()
{
	;
}
