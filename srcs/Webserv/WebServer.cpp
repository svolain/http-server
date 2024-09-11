
#include "WebServer.hpp"
#include <sstream>
#include <fstream>
#include <string>
#include <iterator>
#include <optional>

WebServer::WebServer(const char *m_ipAddress, const char *m_port)
	: TcpListener(m_ipAddress, m_port){
	;
}


/* tellg returns position of file pointer, gives the size of the file in bytes
		  since it was at the end of file */

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
	// oss << "Content-Type: " << contType << "\r\n";
	// oss << "Content-Length: " << (*content).size()  << "\r\n";
  oss << "Transfer-Encoding: chunked" << "\r\n";
	oss << "\r\n";
	// oss << *content;
	std::string output = oss.str();
	size_t size = output.size() + 1;
  std::cout << output << std::endl;
  sendToClient(clientSocket, output.c_str(), size);
}

std::ifstream WebServer::openFile(std::vector<std::string> &parsed, int *errorCode)
{
  std::ifstream file;

	if (parsed.size() < 3 || parsed[0] != "GET"){
    std::cerr << "Unknown request\n";
    return file;
  }

  std::string requestFile = parsed[1];
  std::cout << "the reqFile is " << requestFile << std::endl;
  
  //naive parsing
  if (requestFile != "/" && requestFile.find(".html") == std::string::npos &&  
    requestFile.find(".ico") == std::string::npos) //read bin file
      file.open("." + requestFile, std::ios::binary);
  else //read html file
  {
    if (requestFile == "/")
      requestFile = "index.html";
    file.open("./pages/" + requestFile);
  }
  if (!file.is_open())
    file.open("./pages/404.html");
  else
     *errorCode = 200;
  // Copy elision, c++ 17, not copying the object to get out of function
  return file;
}

void  assignContType(std::string *contType, std::vector<std::string> *parsed)
{
  if ((*parsed)[1].find(".jpg") != std::string::npos)
    (*contType).assign("image/jpeg");
  else if ((*parsed)[1].find(".mp4") != std::string::npos)
    (*contType).assign("video/mp4");
}

void WebServer::onMessageRecieved(const int clientSocket, const char *msg, 
  int length){
	
  (void)length;
	//TODO: check if the header contains "Connection: close"
	std::string content = "<h1>404 Not Found</h1>";
	int errorCode = 404;

  std::istringstream iss(msg);
  std::vector<std::string> parsed
	  ((std::istream_iterator<std::string>(iss)), (std::istream_iterator<std::string>()));
	
  std::string contType = "text/html";
  assignContType(&contType, &parsed);

  std::ifstream file = openFile(parsed, &errorCode);
  composeHeader(clientSocket, errorCode, contType);

  const size_t chunk_size = 8192; // 8KB chunks
  char buffer[chunk_size];
  if (file.is_open()){
    while (file) {
     
      file.read(buffer, chunk_size);
      std::streamsize bytes_read = file.gcount();

      std::cout << "CHUNKING" << std::endl;
      std::cout << buffer << std::endl;

      // Send the size of the chunk in hexadecimal
      std::ostringstream chunk_size_hex;
      chunk_size_hex << std::hex << bytes_read << "\r\n";
      sendToClient(clientSocket, chunk_size_hex.str().c_str(), chunk_size_hex.str().length()); 
      // Send the actual chunk data
      sendToClient(clientSocket, buffer, bytes_read);
      // End the chunk with CRLF
      sendToClient(clientSocket, "\r\n", 2);
    }
    // Send the final zero-length chunk to signal the end
    sendToClient(clientSocket, "0\r\n\r\n", 5);
  }
  else
    sendToClient(clientSocket, "<h1>404 Not Found</h1>", 23);
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

