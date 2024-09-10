
#include "WebServer.hpp"
#include <sstream>
#include <fstream>
#include <string>
#include <iterator>


WebServer::WebServer(const char *m_ipAddress, const char *m_port)
	: TcpListener(m_ipAddress, m_port){
	;
}


/* tellg returns position of file pointer, gives the size of the file in bytes
		  since it was at the end of file */

void WebServer::sendResponse(const int &clientSocket, int errorCode, std::string *content, 
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
	oss << "Content-Type: " << contType << "\r\n";
	oss << "Content-Length: " << (*content).size()  << "\r\n";
	oss << "\r\n";
	oss << *content;
	std::string output = oss.str();
	int size = output.size() + 1;

	int bytesOut = sendToClient(clientSocket, oss.str().c_str(), size);
	if (bytesOut < 0)
		exit (TODO);
	else
		std::cout << "sent " << bytesOut << " bytes out" << std::endl;
}

static int readHTML(std::string requestFile, int *errorCode, std::string *content)
{
  if (requestFile == "/")
      requestFile = "index.html";
  
  std::ifstream htmlFile("./pages/" + requestFile);
  if (!htmlFile.is_open())
  {
    *errorCode = 404;
    return 3;
  }

  content->assign((std::istreambuf_iterator<char>(htmlFile)),
                std::istreambuf_iterator<char>());
  if (*errorCode == 404)
    *errorCode = 200;
  htmlFile.close(); 
  return 0;
}

int WebServer::readRequest(std::string *content, std::vector<std::string> &parsed, int *errorCode)
{
	if (parsed.size() < 3 || parsed[0] != "GET"){
    std::cerr << "Unknown request\n";
    return 1;
  }

  std::string requestFile = parsed[1];
  if (requestFile != "/" && requestFile != "index.html") //read bin file
  {
    std::ifstream binFile(requestFile, std::ios::binary | std::ios::ate);

    if (!(binFile).is_open()){
        std::cerr << "Failed to open bin file\n";
        *errorCode = 500;
        return (readHTML("500.html", errorCode, content));
    }

    content->assign((std::istreambuf_iterator<char>(binFile)),
                    std::istreambuf_iterator<char>());
    binFile.close();
    return 0;
  }
  else //read html file
    return (readHTML(requestFile, errorCode, content));
}

void WebServer::onMessageRecieved(const int clientSocket, const char *msg, int length){
	(void)length;

	//Parse out the document requested
	std::istringstream iss(msg);
	std::vector<std::string> parsed
	((std::istream_iterator<std::string>(iss)), (std::istream_iterator<std::string>()));

	//TODO: check if the header contains "Connection: close"
	std::string content = "<h1>404 Not Found</h1>";
	int errorCode = 404;


	if (readRequest(&content, parsed, &errorCode))
    return ;
  std::string contType = "text/html";
  if (parsed[1].find(".html") != std::string::npos)
  {
    contType.assign("image/jpeg");
  }

  //TODO: check that the request is html or image or other type of file
	sendResponse(clientSocket, errorCode, &content, contType);
  
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

