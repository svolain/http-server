/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpListener.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/05 14:32:57 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/09 15:23:40 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TcpListener.hpp"

#define TODO 123

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

	int yes = 1;
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) 
	{
        if ((listening.fd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
        if (setsockopt(listening.fd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(TODO);
        }
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

struct EventFlag {
    short flag;
    const char* description;
};


EventFlag eventFlags[] = 
{
            {POLLIN, "POLLIN (Data to read)"},
            {POLLOUT, "POLLOUT (Ready for writing)"},
            {POLLERR, "POLLERR (Error)"},
            {POLLHUP, "POLLHUP (Hang-up)"},
            {POLLNVAL, "POLLNVAL (Invalid FD)"},
            {POLLPRI, "POLLPRI (Urgent Data)"}
};

#define TIMEOUT		3000
	/* will be specified by us*/
#define MAXBYTES	16000


int	TcpListener::run()
{
	int 	sock;
	short	revents;
	int		socketsReady; 

	while (1)
	{
		std::vector<pollfd> copyFDs = m_pollFDs;
		/* the socketsReady from poll() 
			is the number of file descriptors with events that occurred. */
		socketsReady = poll(copyFDs.data(), copyFDs.size(), TIMEOUT);
		if (socketsReady == -1)
		{
			perror("poll: ");
			exit(TODO);
		}
		// std::cout << "There are " << socketsReady << " sockets ready" << std::endl;
		if (!socketsReady)
		{
			std::cout << "poll() is waiting..." << std::endl;
			continue;
		}
		for (size_t i = 0; i < copyFDs.size(); i++)
		{
			//for readability
			sock = copyFDs[i].fd;
			revents = copyFDs[i].revents;
			//is it an inbound connection?
			if (sock == listening.fd && (revents & POLLIN))
			{
				/* should we store the address of the connnecting client?
					for now just using nullptr */
				pollfd newClient;
				newClient.fd = accept(listening.fd, nullptr, nullptr);
				if (newClient.fd != -1)
				{
					newClient.events = POLLIN;
					m_pollFDs.push_back(newClient);
					std::cout << "Created a new connection" << std::endl;
				}
				// onClientConnected();
			}
			else if (sock != listening.fd && (revents & POLLHUP))
			{
				std::cout << "hang up" << sock << " with i = " << i  << std::endl;
				close(sock);
				m_pollFDs.erase(m_pollFDs.begin() + i);
				continue;
			}
			else if (sock != listening.fd && (revents & POLLNVAL))
			{
				std::cout << "invalid fd " << sock << " with i = " << i  << std::endl;
				//try to close anyway?
				close(sock);
				m_pollFDs.erase(m_pollFDs.begin() + i);
				continue;
			}
			//there is data to recv
			else if (sock != listening.fd && (revents & POLLIN))
			{
				char buf[MAXBYTES];
				int bytesIn;
				
				/* do it in a loop until recv is 0? 
					would it be considered blocking?*/
				// std::cout << "trying to recv on fd " << sock << " with i = " << i  << std::endl;
				while (1)
				{
					memset(&buf, 0, MAXBYTES);
					bytesIn = recv(sock, buf, MAXBYTES, 0);
					if (bytesIn < 0)
					{
						//drop the client
						//exit for now
						close(sock);
						perror("recv:");
						exit (TODO);
						// m_pollFDs.erase(m_pollFDs.begin() + i);
						// break ;
					}
					else if (bytesIn == 0)
					{
						std::cout << "bytesIn is 0, closing connection " << sock << std::endl;
						close(sock);
						m_pollFDs.erase(m_pollFDs.begin() + i);
						break;
					}
					else 
					{
						//TODO something with the recieved data chunk
						std::cout << "recieved message" << std::endl;
						onMessageRecieved(sock, buf, bytesIn);
					}

				}
			}
			else 
			{
				for (const auto& eventFlag : eventFlags) 
				{
					if (copyFDs[i].revents & eventFlag.flag) 
					{
						std::cout << eventFlag.description << std::endl;
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
