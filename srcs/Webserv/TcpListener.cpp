/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpListener.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/05 14:32:57 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/06 13:19:15 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TcpListener.hpp"

TcpListener::TcpListener()
{
	std::cout << "Default Constructor for TcpListener called" << std::endl;
}

TcpListener::TcpListener(const TcpListener &other)
{
	std::cout << "Copy constuctor for TcpListener called" << std::endl;
	*this = other;
}

TcpListener::~TcpListener()
{
	std::cout << "Destructor for TcpListener called" << std::endl;
}

TcpListener& TcpListener::operator=(const TcpListener& other)
{
	std::cout << "Assignment operator for TcpListener called" << std::endl;
	if (this != &other)
	{
		return *this;
	}
	return *this;
}

TcpListener::TcpListener(const char *m_ipAddress, const char *m_port)
	: m_ipAddress(m_ipAddress), m_port(m_port)
{
	std::cout << "Default Constructor for TcpListener called" << std::endl;
}

int	TcpListener::init()
{
	struct addrinfo hints, *servinfo, *p;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;


	int status = getaddrinfo(m_ipAddress, m_port, &hints, &servinfo);
	if (status  != 0)
	{
		std::cerr << gai_strerror(status) << std::endl;
		return (1);
	}


	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
        if ((listening.fd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        // if (setsockopt(listening, SOL_SOCKET, SO_REUSEADDR, &yes,
        //         sizeof(int)) == -1) {
        //     perror("setsockopt");
        //     exit(1);
        // }
        if (bind(listening.fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(listening.fd);
            perror("server: bind");
            continue;
        }
        break;
    };

	if (p == NULL)
	{
		std::cerr << "server: failed to bind\n";
		return (2);
	}

	// fcntl(listening.fd, F_SETFL, O_NONBLOCK);
	
	freeaddrinfo(servinfo);

	#define BACKLOG 10

	if (listen(listening.fd, BACKLOG) == -1)
	{
		perror("listen:");
		return (3);
	}

	/* POLLIN is read-ready, 
		POLLPRI is for out-of-band data, 
		is it related to building a packet from multiple packets?*/
	listening.events = POLLIN | POLLPRI;
	m_pollFDs.push_back(listening);


	return (0);
}


int	TcpListener::run()
{
	#define TIMEOUT 1000
	/* will be specified by the  */
	#define MAXBYTES 16000


	while (1)
	{
			/* if poll() makes unwanted changes to the main vector */
		std::vector<pollfd> copyFDs = m_pollFDs;
		// for (size_t i = 0; i < m_pollFDs.size(); i++)
		// {
		// 	std::cout << "revents for " << i << " " << m_pollFDs[i].revents << std::endl;
		// }
		/* the socketsReady from poll() 
			is the number of file descriptors with events that occurred. */
		int socketsReady = poll(copyFDs.data(), copyFDs.size(), TIMEOUT);
		std::cout << "There are " << socketsReady << " sockets ready" << std::endl;
		// std::cout << "after poll" << std::endl;
		for (size_t i = 0; i < copyFDs.size(); i++)
		{
			//for readability
			int sock = copyFDs[i].fd;
			std::cout << sock << std::endl;
			//is it an inbound connection?
			if (sock == listening.fd && (copyFDs[i].revents & POLLIN))
			{
				/* should we store the address of the connnecting client?
					for now just using nullptr */
				pollfd newClient;
				std::cout << "before accept" << std::endl;
				newClient.fd = accept(listening.fd, nullptr, nullptr);
				if (newClient.fd != -1)
				{
					newClient.events = POLLIN;
					m_pollFDs.push_back(newClient);
					std::cout << "Created a new connection" << std::endl;
				}
				else 
					// perror("Accept:");
				std::cout << "after accept" << std::endl;

				
				// onClientConnected();
			}
			else if (sock != listening.fd)
			{
				char buf[MAXBYTES];
				int bytesIn;
				
				/* do it in a loop until recv is 0? */
				while (1)
				{
					memset(&buf, 0, MAXBYTES);
					bytesIn = recv(sock, buf, MAXBYTES, 0);
					if (bytesIn < 0)
					{
						//drop the client
						close(sock);
						perror("recv:");
						exit (11);
						m_pollFDs.erase(m_pollFDs.begin() + i);
						break ;
					}
					else if (bytesIn == 0)
					{
						break;
					}
					else 
					{
						//TODO something with the recieved data chunk
						std::cout << "recieved message" << std::endl;
						onMessageRecieved(sock, buf, bytesIn);
						break ;
					}

				}
			}
		}	
	}

	m_pollFDs.clear();
	//closing the listening socket
	close(listening.fd);

	std::cout << "--- Shutting down the server ---" << std::endl;
	
	return (0);
}

int TcpListener::sendToClient(int clientSocket, const char *msg, int length)
{
	return (send(clientSocket, msg, length, 0));
}
