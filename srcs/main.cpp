/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vsavolai <vsavolai@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/05 15:17:35 by shatilovdr        #+#    #+#             */
/*   Updated: 2024/09/09 16:30:01 by vsavolai         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Webserv.hpp"

#include <sys/socket.h>
#include <netdb.h>         // For getaddrinfo() and freeaddrinfo()
#include <unistd.h>        // For close()
#include <iostream> 
#include <fcntl.h>       
#include <poll.h>
#include <vector>
#include <signal.h>

bool run = true;

void signalHandler(int signum) {
    std::cout << "\nSignal " << signum << " received" << std::endl;
    run = false;
}

int main() {

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    int listeningSocket;
    struct addrinfo hints, *res;
    
    // Zero out the hints structure and assing values to needed fields
    hints = {};
    hints.ai_family = AF_INET;       // AF_INET for IPv4
    hints.ai_socktype = SOCK_STREAM; // Stream socket (TCP)
    hints.ai_flags = AI_PASSIVE;     // Use the address for binding

    // GetAddressInfo sets localhost (127.0.0.1) and port 8080 in correct byte order, 
    // later localhost and port is replaced with whatever we get from config file
    if (getaddrinfo("127.0.0.1", "8080", &hints, &res) != 0) {
        std::cerr << "getaddrinfo failed" << std::endl;
        return 1;
    }

    // Create a socket using the information from getaddrinfo()
    listeningSocket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (listeningSocket < 0) {
        std::cerr << "Socket creation failed" << std::endl;
        freeaddrinfo(res);
        return 1;
    }

    // Enable the SO_REUSEADDR option to reuse the address
    int opt = 1;
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "setsockopt(SO_REUSEADDR) failed" << std::endl;
        close(listeningSocket);
        freeaddrinfo(res);
        return 1;
    }

    // Set socket to non-blocking mode
    int flags = fcntl(listeningSocket, F_GETFL, 0);
    if (fcntl(listeningSocket, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "Failed to set non-blocking mode" << std::endl;
        close(listeningSocket);
        freeaddrinfo(res);
        return 1;
    }

    // Bind the socket to the address and port
    if (bind(listeningSocket, res->ai_addr, res->ai_addrlen) < 0) {
        std::cerr << "Binding socket failed" << std::endl;
        close(listeningSocket);
        freeaddrinfo(res);
        return 1;
    }

    // Start listening for connections
    listen(listeningSocket, 5);
    std::cout << "Server listening on 127.0.0.1:8080" << std::endl;

    // Make vector to store poll structures, make pollfd for socket and then add into vector
    std::vector<pollfd> pollfds;
    pollfd serverPollFd = {listeningSocket, POLLIN, 0};
    pollfds.push_back(serverPollFd);

    // Timeout to 10 seconds (10000 milliseconds)
    int timeout = 10000;

    while (run) {
        //Use poll to iterate throught vector to check socket activity, 
        //timeout so it wont wait indefinitely
        int pollCount = poll(pollfds.data(), pollfds.size(), timeout);
        if (pollCount < 0 && run == true) {
            std::cerr << "Poll failed" << std::endl;
            close(listeningSocket);
            freeaddrinfo(res);
            return 1;
        } else if (pollCount == 0) {
            std::cout << "No activity within timeout" << std::endl;
            continue;
        }

        // Check the listeningsocket for incoming connections
        if (pollfds[0].revents & POLLIN) {
            int clientSocket = accept(listeningSocket, nullptr, nullptr);
            if (clientSocket >= 0) {
                // Set the new clientSocket to non-blocking
                fcntl(clientSocket, F_SETFL, O_NONBLOCK);

                // Add  to the poll vector
                pollfd clientPollFd = {clientSocket, POLLIN, 0};
                pollfds.push_back(clientPollFd);
                std::cout << "New client connected" << std::endl;
            } 

            //itarate client sockets
            for (size_t i = 1; i < pollfds.size(); ++i) {
                if (pollfds[i].revents & POLLIN) {
                    char buffer[1024] = {0};
                    int bytesReceived = recv(pollfds[i].fd, buffer, sizeof(buffer), 0);
                    
                    if (bytesReceived > 0) {
                        buffer[bytesReceived] = '\0';
                        std::cout << "Message from client: " << buffer << std::endl;
                        //Response to be changed into html 
                         std::string response = "";
                        send(pollfds[i].fd, response.c_str(), response.size(), 0);
                    } else if (bytesReceived == 0 || (bytesReceived < 0 && errno != EWOULDBLOCK)) {
                        // If no bytes received or error, close the client socket
                        std::cout << "Client disconnected" << std::endl;
                        close(pollfds[i].fd);

                        // Remove the client from the poll list
                        pollfds.erase(pollfds.begin() + i);
                        --i; // Adjust index to account for removed element
                    }
                }
            }
        }

    }
    //close still open sockets and free allocated getAdress struct
    for (size_t i = 1; i < pollfds.size(); ++i) {
        close(pollfds[i].fd);
    }
    close(listeningSocket);
    freeaddrinfo(res);
    std::cout << "Server shutdown" << std::endl;

    return 0;
}

