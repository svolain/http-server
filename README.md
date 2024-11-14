# HTTP - Server

This is a HTTP server project designed to handle HTTP/1.1 requests and responses across modern web browsers. 
Inspired by the architecture and configuration styles of the NGINX server, this project introduces key networking 
and server management concepts such as socket programming, non-blocking I/O, request parsing, and client-server 
communication.

## Key Features

-Configuration-Driven: Webserv accepts a configuration file that defines server behavior, including host and port settings, error pages, client body size limits, and route-specific rules.

-Non-Blocking I/O: The server manages multiple client connections using a single event polling mechanism (poll, epoll, or kqueue), ensuring efficient handling of concurrent requests without blocking.

-HTTP Methods: Supports GET, POST, and DELETE HTTP methods, allowing static file serving, file uploads, and resource deletion.

-CGI Support: Executes CGI scripts for dynamic content generation based on specific route configurations.

-Error Handling: Returns accurate HTTP status codes with customizable error pages for common errors (404, 500, etc.).

-Static Site Hosting: Enables serving static websites from a specified root directory, with options for directory listing and default file handling.

-Redirection and Uploads: Supports HTTP redirection and file upload configurations for specific routes.

## Tech Requirements

-Single Polling Mechanism: Only one poll (or equivalent) call is used for all I/O operations to avoid blocking and optimize 
resource use.

-Socket and Network Programming: Utilizes essential network functions such as socket, bind, listen, accept, recv, and send to establish and manage client connections.

-Error-Resilient Design: The server is tested for stability under high-load conditions to ensure that it remains responsive and resilient.

## Install & Run

```
git clone git@github.com:svolain/http-server.git
cd http-server
make
./webserv [configuration_file]
```
If no configuration file is provided with ./webserv call the program will use it's default configuration defined in conf directory of this repository.
