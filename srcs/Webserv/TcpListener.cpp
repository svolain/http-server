/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TcpListener.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/05 14:32:57 by klukiano          #+#    #+#             */
/*   Updated: 2024/09/13 14:01:55 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TcpListener.hpp"

TcpListener::TcpListener(const char *m_ipAddress, const char *m_port)
	: m_ipAddress(m_ipAddress), m_port(m_port){
	;
}


int	TcpListener::init(){
	struct addrinfo hints{}, *servinfo;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int status = getaddrinfo(m_ipAddress, m_port, &hints, &servinfo);
	if (status  != 0){
		std::cerr << gai_strerror(status) << std::endl;
		return (1);
	}

	if ((listening.fd = socket(servinfo->ai_family, servinfo->ai_socktype,
			servinfo->ai_protocol)) == -1) {
		std::cerr << "server: socket() error"  << std::endl; 
		return 1;
	}
	/* SO_REUSEADDR for TCP to handle the case when the server shuts down
		and we can't bind to the same socket if there's data left
		port goes into a TIME_WAIT state otherwise*/
	int yes = 1;
	if (setsockopt(listening.fd, SOL_SOCKET, SO_REUSEADDR, &yes,
			sizeof(int)) == -1) {
		std::cerr << "server: setsockopt() error"  << std::endl; 
		return (2);
	}
	if (bind(listening.fd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
		close(listening.fd);
		std::cerr << "server: bind() error"  << std::endl; 
		return (3);
	}
	// fcntl(listening.fd, F_SETFL, O_NONBLOCK);
	
	freeaddrinfo(servinfo);

	#define BACKLOG 10

	if (listen(listening.fd, BACKLOG) == -1){
		std::cerr << "server: listen() error"  << std::endl; 
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


EventFlag eventFlags[] = {
            {POLLIN, "POLLIN (Data to read)"},
            {POLLOUT, "POLLOUT (Ready for writing)"},
            {POLLERR, "POLLERR (Error)"},
            {POLLHUP, "POLLHUP (Hang-up)"},
            {POLLNVAL, "POLLNVAL (Invalid FD)"},
            {POLLPRI, "POLLPRI (Urgent Data)"}
};

#define TIMEOUT		5000
	/* will be specified by us*/
#define MAXBYTES	16000


int	TcpListener::run(){
	while (1)
	{
		std::vector<pollfd> copyFDs = m_pollFDs;
		/* the socketsReady from poll() 
			is the number of file descriptors with events that occurred. */
		int socketsReady = poll(copyFDs.data(), copyFDs.size(), TIMEOUT);
		if (socketsReady == -1){
			perror("poll: ");
			exit(TODO);
		}
		if (!socketsReady){
			std::cout << "poll() is closing connections on timeout..." << std::endl;
			for (size_t i = 1; i < m_pollFDs.size(); i ++)
			{
				close(m_pollFDs[i].fd);
				m_pollFDs.erase(m_pollFDs.begin() + i);
			}	
			continue;
		}
		for (size_t i = 0; i < copyFDs.size(); i++){
			//for readability
			int sock = copyFDs[i].fd;
			short revents = copyFDs[i].revents;
			//is it an inbound connection?
			if (sock == listening.fd){
				if (revents & POLLIN){
				/* should we store the address of the connnecting client?
					for now just using nullptr */
				pollfd newClient;
				newClient.fd = accept(listening.fd, nullptr, nullptr);
				if (newClient.fd != -1){
					newClient.events = POLLIN;
					m_pollFDs.push_back(newClient);
					// std::cout << "Created a new connection" << std::endl;
				}
				}
				// onClientConnected();
			}
			else if (sock != listening.fd && (revents & POLLHUP)){
				std::cout << "hang up" << sock << " with i = " << i  << std::endl;
				close(sock);
				m_pollFDs.erase(m_pollFDs.begin() + i);
				continue;
			}
			else if (sock != listening.fd && (revents & POLLNVAL))
			{
				std::cout << "invalid fd " << sock << " with i = " << i  << std::endl;
				//try to close anyway
				close(sock);
				m_pollFDs.erase(m_pollFDs.begin() + i);
				continue;
			}
			//there is data to recv
			else if (sock != listening.fd && (revents & POLLIN)){
				char  buf[MAXBYTES];
				int   bytesIn;
				fcntl(sock, F_SETFL, O_NONBLOCK);
				/* do it in a loop until recv is 0? 
					would it be considered blocking?*/
				while (1){
					//TODO: add a Timeout timer for the client connection
					memset(&buf, 0, MAXBYTES);
					bytesIn = recv(sock, buf, MAXBYTES, 0);
					std::cout << bytesIn << std::endl;
					if (bytesIn < 0){
						close(sock);
						m_pollFDs.erase(m_pollFDs.begin() + i);
						perror("recv -1:");
						break ;
					}
					/* with the current break after onMessageRecieved
						we can't get here */
					else if (bytesIn == 0){
						close(sock);
						m_pollFDs.erase(m_pollFDs.begin() + i);
						break;
					}
					else {
						onMessageRecieved(sock, buf, bytesIn);
						/* If the request is maxbytes we should not break the connection here */
						break ;
					}

				}
			}
			/* An unspecified event will trigget this loop 
				to see which flag is in the revents*/
			else {
				for (const auto& eventFlag : eventFlags) {
					if (revents & eventFlag.flag) 
						std::cout << eventFlag.description << std::endl;
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

int TcpListener::sendToClient(const int clientSocket, const char *msg, int length){
	return (send(clientSocket, msg, length, 0));
}


TcpListener::TcpListener(){
	;
}

TcpListener::~TcpListener(){
	;
}