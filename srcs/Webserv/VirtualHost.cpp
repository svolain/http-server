/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   VirtualHost.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: klukiano <klukiano@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/15 17:35:52 by  dshatilo         #+#    #+#             */
/*   Updated: 2024/10/02 17:22:32 by klukiano         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "VirtualHost.hpp"
#include "ConnectInfo.hpp"

extern bool show_request;
extern bool show_response;

#define MAXBYTES  8192

int VirtualHost::ParseHeader(ConnectInfo* fd_info, pollfd& poll) {

  char                buf[MAXBYTES]{};
  int                 bytesIn;
  HttpParser*         parser = fd_info->get_parser();

  int fd = fd_info->get_fd();
  bytesIn = recv(fd, buf, MAXBYTES, 0);
  if (bytesIn < 0)
    return 1;
  else if (bytesIn == 0) {
    /* When a stream socket peer has performed an orderly shutdown, the
      return value will be 0 (the traditional "end-of-file" return) */
    return 2;
  }
  else if (bytesIn == MAXBYTES)
    std::cout << "MAXBYTES on recv. Check if the header is too long" << std::endl;

  
  if (show_request)
    std::cout << "\nthe whole request is:\n" << buf << std::endl;
  
  if (!parser->ParseRequest(buf))
    std::cout << "false on ParseRequest returned" << std::endl;

  if (fd_info->get_vhost() == nullptr)
    fd_info->AssignVHost();
  
  /* If the message didnt fit into MAXBYTES then dont set the POLLOUT yet,
    let the WriteBody do that */
  if (parser->get_is_chunked() == true)
    fd_info->set_is_parsing_body(true);
  else
    poll.events = POLLOUT;
  /* Set to true if we want to read the body 
    If the whole message fit into MAXBYTES then dont set it to true*/
  return 0;
}

int VirtualHost::WriteBody(ConnectInfo* fd_info, pollfd& poll) {

  std::array<char, MAXBYTES>
    &request_body = fd_info->get_parser()->get_request_body();
  size_t              body_size = request_body.size();
  int                 bytesIn;
  size_t              request_size = 0;


  int fd = fd_info->get_fd();
  bytesIn = recv(fd, request_body.data() + body_size, MAXBYTES, 0);
  if (bytesIn < 0)
    return 1;
  else if (bytesIn == 0) {
    /* When a stream socket peer has performed an orderly shutdown, the
      return value will be 0 (the traditional "end-of-file" return) */
    return 2;
  }
  else if (bytesIn == MAXBYTES)
    std::cout << "the header is too long! handle this" << std::endl;

  request_size += bytesIn;
  poll.events = POLLOUT;
  return 0;
}

bool VirtualHost::ParseBody(std::vector<char> buf, size_t bytesIn, std::map<std::string, std::string> headers) {
  if (headers["transfer-encoding"] == "chunked") {
      ;
    }
    (void)bytesIn;
    (void)buf;
		/*std::string eoc = "0\r\n\r\n";
    check if the chunk is received in whole, if not return to receiving next chunk
    if (!oss.find(eoc));
      return;
    when all the chunks are received, the chunk characters need to be removed
    UnChunkBody();
	}*/
	// if (/*check if content-length fully read*/)
	// 	return ;
	// else if (headers["content-type"].find("multipart/form-data") != std::string::npos){
	// }

	return true;        
}

void VirtualHost::OnMessageRecieved(ConnectInfo *fd_info, pollfd &poll){

  if (show_response)
    std::cout << "--- entering OnMessageRecieved ---" << std::endl;

  if (fd_info->get_is_sending() == false){
      SendHeader(fd_info);
      fd_info->set_is_sending(true);
  }
  else
    SendChunkedBody(fd_info, poll);
    
  if (show_response)
    std::cout << "----- leaving OnMessageRecieved -----" << std::endl;
}

void VirtualHost::SendHeader(ConnectInfo *fd_info){
  HttpParser *parser = fd_info->get_parser();
  std::string resource_path = parser->get_resource_path();

  if (show_request)
    std::cout << "the resource path is " << resource_path << std::endl;
  if (show_response)
    std::cout << "the error code from parser is " << parser->get_error_code() << std::endl;
    
  HttpResponse response;
  response.set_error_code_(parser->get_error_code());
  response.AssignContType(parser->get_resource_path());
  std::ifstream& file = fd_info->get_file();
  response.OpenFile(resource_path, file);
  response.ComposeHeader();

  if (show_response){
    std::cout << "\n------response header------" << std::endl;
    std::cout << response.get_header_() << std::endl;
    std::cout << "-----end of response header------\n" << std::endl;
  }
  
  SendToClient(fd_info->get_fd(), response.get_header_().c_str(), response.get_header_().size());
}

void VirtualHost::SendChunkedBody(ConnectInfo* fd_info, pollfd &poll)
{
  /* Nginx will not try to save the state of the previous transmission and retry later. 
      It handles each request-response transaction independently. 
      If the connection breaks, a client would need to send a new request to get the content again. 
      Return the file position back to 0*/

  HttpParser *parser = fd_info->get_parser();    
  std::string resource_path = parser->get_resource_path();
  std::ifstream& file = fd_info->get_file();
  HttpResponse response;
  response.OpenFile(resource_path, file);
  int client_socket = fd_info->get_fd();
  if (file.is_open()){
    if (SendOneChunk(client_socket, file) == 0){
      return ;
    } 
    if (SendToClient(client_socket, "0\r\n\r\n", 5) == -1)
      perror("send 2:");
  }
  else
    if (SendToClient(client_socket, "<h1>404 Not Found</h1>", 23) == -1)
      perror("send 3:");
  file.close();
  poll.events = POLLIN;
  fd_info->set_is_sending(false);
  
  if (show_response)
    std::cout << "\n-----response sent-----\n" << std::endl;
}

int VirtualHost::SendOneChunk(int client_socket, std::ifstream &file)
{
  std::streamsize bytes_read;
  const int chunk_size = 1024;
  char buffer[chunk_size]{};

  file.read(buffer, chunk_size);
  bytes_read = file.gcount(); 
  if (bytes_read == -1){
    std::cout << "bytes_read returned -1" << std::endl;
    return 1;
  }
  std::ostringstream chunk_size_hex;
  chunk_size_hex << std::hex << bytes_read << "\r\n";
  if (SendToClient(client_socket, chunk_size_hex.str().c_str(), 
    chunk_size_hex.str().length()) == -1 || 
    SendToClient(client_socket, buffer, bytes_read) == -1 ||
    SendToClient(client_socket, "\r\n", 2) == -1){
      perror("send :");
      return 1;
  }
  if (show_response)
    std::cout << "sent " << bytes_read  << std::endl;
  if (bytes_read < chunk_size)
    return 1;
  return 0;
}

int VirtualHost::SendToClient(const int client_socket, const char *msg, int length){
  int bytes_sent;
  /* https://stackoverflow.com/questions/108183/how-to-prevent-sigpipes-or-handle-them-properly */
  bytes_sent = send(client_socket, msg, length, MSG_NOSIGNAL);
  if (bytes_sent != length)
    return -1;
  return bytes_sent;
}

std::string VirtualHost::get_name() {
  return name_;
}

void VirtualHost::set_name(std::string& name) {
  name_ = name;
}

void VirtualHost::set_size(std::string& size) {
  client_max_body_size_ = std::stoi(size);
  client_max_body_size_ *= (size.back() == 'M' ? 1048576 : 1024);
}

void VirtualHost::set_error_page(std::string& code, std::string& path) {
  error_pages_[code] = path;
}

void VirtualHost::set_location(std::string& path, Location& location) {
  locations_[path] = location;
}

size_t VirtualHost::get_max_body_size(){
  return client_max_body_size_;
}