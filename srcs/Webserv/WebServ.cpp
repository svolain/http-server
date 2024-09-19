/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServ.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/06 15:30:31 by dshatilo          #+#    #+#             */
/*   Updated: 2024/09/19 16:34:43 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServ.hpp"
#include "HttpResponse.hpp"
#include "HttpParser.hpp"


extern bool showResponse;
extern bool showRequest;

#define TODO 123

WebServ::WebServ(char* conf) : conf_(conf != nullptr ? conf : DEFAULT_CONF) {}

int WebServ::init() {

  ConfigParser parser;

  if (parser.parseConfig(sockets_)) 
    return 1;
    
	struct addrinfo hints{}, *servinfo;
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int status = getaddrinfo(ipAddress_, port_, &hints, &servinfo);
	if (status  != 0){
		std::cerr << gai_strerror(status) << std::endl;
		return (1);
	}

	if ((listening_.fd = socket(servinfo->ai_family, servinfo->ai_socktype,
			servinfo->ai_protocol)) == -1) {
		std::cerr << "server: socket() error"  << std::endl; 
		return 1;
	}
	/* SO_REUSEADDR for TCP to handle the case when the server shuts down
		and we can't bind to the same socket if there's data left
		port goes into a TIME_WAIT state otherwise*/
	int yes = 1;
	if (setsockopt(listening_.fd, SOL_SOCKET, SO_REUSEADDR, &yes,
			sizeof(int)) == -1) {
		std::cerr << "server: setsockopt() error"  << std::endl; 
		return (2);
	}
	if (bind(listening_.fd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
		close(listening_.fd);
		std::cerr << "server: bind() error"  << std::endl; 
		return (3);
	}
	// fcntl(listening_.fd, F_SETFL, O_NONBLOCK);
	
	freeaddrinfo(servinfo);

	#define BACKLOG 10

	if (listen(listening_.fd, BACKLOG) == -1){
		std::cerr << "server: listen() error"  << std::endl; 
		return (3);
	}

	/* POLLIN is read-ready, 
		POLLPRI is for out-of-band data, 
		is it related to building a packet from multiple packets?*/
	listening_.events = POLLIN | POLLPRI ;
	pollFDs_.push_back(listening_);


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


void WebServ::run() {
  std::cout << "Server is ready.\n";
	while (1)
	{
		std::vector<pollfd> copyFDs = pollFDs_;
		/* the socketsReady from poll() 
			is the number of file descriptors with events that occurred. */
		int socketsReady = poll(copyFDs.data(), copyFDs.size(), TIMEOUT);
		if (socketsReady == -1){
			perror("poll: ");
			exit(TODO);
		}
		if (!socketsReady){
			std::cout << "poll() is closing connections on timeout..." << std::endl;
			for (size_t i = 1; i < pollFDs_.size(); i ++)
			{
				close(pollFDs_[i].fd);
				pollFDs_.erase(pollFDs_.begin() + i);
			}	
			continue;
		}
		for (size_t i = 0; i < copyFDs.size(); i++){
			//for readability
			int sock = copyFDs[i].fd;
			short revents = copyFDs[i].revents;
			//is it an inbound connection?
			if (sock == listening_.fd){
				if (revents & POLLIN){
				/* should we store the address of the connnecting client?
					for now just using nullptr */
				pollfd newClient;
				newClient.fd = accept(listening_.fd, nullptr, nullptr);
				if (newClient.fd != -1){
					newClient.events = POLLIN;
					pollFDs_.push_back(newClient);
					// std::cout << "Created a new connection" << std::endl;
				}
				}
				// onClientConnected();
			}
			else if (sock != listening_.fd && (revents & POLLHUP)){
				std::cout << "hang up" << sock << " with i = " << i  << std::endl;
				close(sock);
				pollFDs_.erase(pollFDs_.begin() + i);
				continue;
			}
			else if (sock != listening_.fd && (revents & POLLNVAL))
			{
				std::cout << "invalid fd " << sock << " with i = " << i  << std::endl;
				//try to close anyway
				close(sock);
				pollFDs_.erase(pollFDs_.begin() + i);
				continue;
			}
			//there is data to recv
			else if (sock != listening_.fd && (revents & POLLIN)){
				char  buf[MAXBYTES];
				int   bytesIn;
				fcntl(sock, F_SETFL, O_NONBLOCK);
				/* do it in a loop until recv is 0? 
					would it be considered blocking?*/
				while (1){
					//TODO: add a Timeout timer for the client connection
					memset(&buf, 0, MAXBYTES);
					bytesIn = recv(sock, buf, MAXBYTES, 0);
					if (bytesIn < 0){
						close(sock);
						pollFDs_.erase(pollFDs_.begin() + i);
						perror("recv -1:");
						break ;
					}
					/* with the current break after on_message_recieved
						we can't get here */
					else if (bytesIn == 0){
						close(sock);
						pollFDs_.erase(pollFDs_.begin() + i);
						break;
					}
					else {
						on_message_recieved(sock, buf, bytesIn, revents);
						if (!recv(sock, buf, 0, 0))
							std::cout << "Client closed the connection" << std::endl;
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
			if (sock != listening_.fd && (revents & POLLOUT)){
				std::cout << "ready for write" << std::endl;
			}
	}	

	pollFDs_.clear();
	//closing the listening_ socket
	close(listening_.fd);

	std::cout << "--- Shutting down the server ---" << std::endl;

}

}


void WebServ::on_message_recieved(const int clientSocket, const char *msg, 
  int bytesIn, short revents){
	
  (void)bytesIn;
  (void)revents;
  std::cout << "--- entering on_message_recieved ---" << std::endl;
  /* TODO if bytesIn == MAXBYTES then recv until the whole message is sent 
    see 100 Continue status message
    https://www.w3.org/Protocols/rfc2616/rfc2616-sec8.html#:~:text=Requirements%20for%20HTTP/1.1%20origin%20servers%3A*/

	//TODO: check if the header contains "Connection: close"

  // std::istringstream iss(msg);
  // std::vector<std::string> parsed
	//   ((std::istream_iterator<std::string>(iss)), (std::istream_iterator<std::string>()));
  // std::cout << parsed[1] << std::endl;
  
  HttpParser parser;
  HttpResponse response;


  if (!parser.parseRequest(msg))
    std::cout << "false on parseRequest returned" << std::endl;
  std::cout << "\nrequestBody:\n" << parser.get_request_body() << std::endl;

  std::cout << "the err code is " << parser.get_error_code() << std::endl;
  response.set_error_code_(parser.get_error_code());
  
  if (showRequest)
    std::cout << "the resource path is " << parser.get_resource_path() << std::endl;
  response.assign_cont_type_(parser.get_resource_path()); 
  response.open_file(parser.get_resource_path());
  response.compose_header();
  if (showResponse)
  {
    std::cout << "\n------response header------" << std::endl;
    std::cout << response.get_header_() << std::endl;
    std::cout << "-----end of response header------\n" << std::endl;
  }
  send_to_client(clientSocket, response.get_header_().c_str(), response.get_header_().size());

  send_chunked_response(clientSocket, response.get_file_());
}

void WebServ::send_chunked_response(int clientSocket, std::ifstream &file)
{
  const int chunk_size = 1024;
   
  char buffer[chunk_size]{};
  // int i = 0;
  /* TODO: check if the handling of SIGINT on send error is needed  
    It would sigint on too many failed send() attempts*/
  if (file.is_open()){
    std::streamsize bytesRead;
    
    while (file) {
      file.read(buffer, chunk_size);
      bytesRead = file.gcount(); 
      if (bytesRead == -1)
      {
        std::cout << "bytesRead returned -1" << std::endl;
        break;
      }
      std::ostringstream chunk_size_hex;
      chunk_size_hex << std::hex << bytesRead << "\r\n";
      if (send_to_client(clientSocket, chunk_size_hex.str().c_str(), chunk_size_hex.str().length()) == -1 || 
        send_to_client(clientSocket, buffer, bytesRead) == -1 ||
        send_to_client(clientSocket, "\r\n", 2) == -1){
          perror("send :");
          break ;
      }
      // i ++;
    }
    // std::cout << "sent a chunk " << i << " times and the last one was " << bytesRead << std::endl;
    if (send_to_client(clientSocket, "0\r\n\r\n", 5) == -1)
      perror("send 2:");
  }
  else
    if (send_to_client(clientSocket, "<h1>404 Not Found</h1>", 23) == -1)
      perror("send 3:");
  file.close();
  std::cout << "\n-----response sent-----\n" << std::endl;
}

void WebServ::onClientConnected()
{
	;
}

void WebServ::onClientDisconected()
{
	;
}

int WebServ::send_to_client(const int clientSocket, const char *msg, int length){
	return (send(clientSocket, msg, length, 0));
}

