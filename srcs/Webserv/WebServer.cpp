
#include "WebServer.hpp"
#include "HttpResponse.hpp"
#include "HttpParser.hpp"
#include <sstream>
#include <fstream>
#include <string>
#include <iterator>
// #include <optional>

WebServer::WebServer(const char *m_ipAddress, const char *m_port)
	: TcpListener(m_ipAddress, m_port){
	;
}

void WebServer::composeHeader(const int &clientSocket, int errorCode, 
	const std::string contType){

  //TODO do some kind of a table duple lookup for error codes  messages
  std::string errorCodeMessage;
  switch (errorCode)
  {
    case 500:
      errorCodeMessage = "500 Internal Server Error";
      break;

    case 200:
      errorCodeMessage = "200 OK";
      break;
    
    default:
      errorCodeMessage = "404 Not Found";
      break;
  }

  std::ostringstream oss;
	oss << "HTTP/1.1 " << errorCodeMessage << "\r\n";
  (void)contType;
	oss << "Content-Type: " << contType << "\r\n";
	// oss << "Content-Length: " << (*content).size()  << "\r\n";
  oss << "Transfer-Encoding: chunked" << "\r\n";
	oss << "\r\n";
	// oss << *content;
	std::string output = oss.str();
  std::cout << output << std::endl;
  sendToClient(clientSocket, oss.str().c_str(), output.size());
}

std::ifstream WebServer::openFile(std::string resourcePath)
{
  std::ifstream file;

  std::cout << "the resourcePath is " << resourcePath << std::endl;
  
  //naive parsing
  if (resourcePath != "/" && resourcePath.find(".html") == std::string::npos &&  
    resourcePath.find(".ico") == std::string::npos) //read bin file
      file.open("." + resourcePath, std::ios::binary);
  else //read html file
  {
    if (resourcePath == "/")
      resourcePath = "index.html";
    file.open("./pages/" + resourcePath);
  }
  if (!file.is_open())
    file.open("./pages/404.html");

  //TODO: change the error code when the methd is gonna be of the responese class

  // Copy elision, c++ 17, not copying the object to get out of function
  return file;
}


void WebServer::onMessageRecieved(const int clientSocket, const char *msg, 
  int bytesIn){
	
  (void)bytesIn;
  /* TODO if bytesIn == MAXBYTES then recv until the whole message is sent 
    see 100 Continue status message
    https://www.w3.org/Protocols/rfc2616/rfc2616-sec8.html#:~:text=Requirements%20for%20HTTP/1.1%20origin%20servers%3A*/

	//TODO: check if the header contains "Connection: close"

  // std::istringstream iss(msg);
  // std::vector<std::string> parsed
	//   ((std::istream_iterator<std::string>(iss)), (std::istream_iterator<std::string>()));
  
  HttpParser parser;
  HttpResponse response;
  if (!parser.parseRequest(msg))
    std::cout << "false on parseRequest returned" << std::endl;
  response.assignContType(parser.getResourcePath()); 

  std::ifstream file = openFile(parser.getResourcePath());
  composeHeader(clientSocket, parser.getErrorCode(), response.getContType());

  response.setErrorCode(123); // temp to shut the compiler

  const int chunk_size = 8192; // 8KB chunks
  char buffer[chunk_size]{};
  if (file.is_open()){
    while (file) {
      file.read(buffer, chunk_size);
      
      std::streamsize bytes_read = file.gcount(); 
      if (bytes_read == -1)
      {
        std::cout << "bytes_read returned -1" << std::endl;
        sendToClient(clientSocket, "0\r\n\r\n", 5);
        break;
      }
      // Send the size of the chunk in hexadecimal
      std::ostringstream chunk_size_hex;
      chunk_size_hex << std::hex << bytes_read << "\r\n";
      if (sendToClient(clientSocket, chunk_size_hex.str().c_str(), chunk_size_hex.str().length()) == -1)
        perror("send 1:");
      if (sendToClient(clientSocket, buffer, bytes_read) == -1)
        perror("send 2:");
      // End the chunk with CRLF
      if (sendToClient(clientSocket, "\r\n", 2) == -1)
        perror("send 3:");

    }
    // Send the final zero-length chunk to signal the end
    if (sendToClient(clientSocket, "0\r\n\r\n", 5) == -1)
      perror("send 4:");
  }
  else
    if (sendToClient(clientSocket, "<h1>404 Not Found</h1>", 23) == -1)
      perror("send 5:");
  file.close(); 
}

void WebServer::onClientConnected()
{
	;
}

void WebServer::onClientDisconected()
{
	;
}

// void WebServer::sendVideo(int clientSocket, const char *msg, int length)
// {
// 	// if (parsed.size() >= 3 && parsed[0] == "GET")
// 	// {	
// 		const char *videoPath = "./pages/video.mp4";  // Path to the video file
// 		/* binary: for non text files, skips newlines
// 			ate: poistion the pointer At the End of file upon opening for determening the size*/
//     	std::ifstream videoFile(videoPath, std::ios::binary | std::ios::ate);
// 		if (!videoFile.is_open())
// 			std::cerr << "Failed to open file\n";
// 		/* tellg returns position of file pointer, gives the size of the file in bytes
// 			since it was at the end of file */
// 		std::streampos fileSize = videoFile.tellg();
// 		videoFile.seekg(0, std::ios::beg);
// 	// }

// 	oss << "Content-Type: video/mp4\r\n";
	// char fileBuffer[4096];
// 	oss << "Content-Length: " << std::to_string(fileSize)  << "\r\n";
	// while (videoFile.read(fileBuffer, sizeof(fileBuffer))) {
    //     send(clientSocket, fileBuffer, sizeof(fileBuffer), 0);
	// 	std::cout << "here22" << std::endl;
    // }
    // if (videoFile.gcount() > 0) {
    //     send(clientSocket, fileBuffer, videoFile.gcount(), 0);
    // }
	//  videoFile.close();
// }

