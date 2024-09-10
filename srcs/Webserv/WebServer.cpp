
#include "WebServer.hpp"
#include <sstream>
#include <fstream>
#include <string>
#include <iterator>



WebServer::WebServer()
{
	std::cout << "Default Constructor for WebServer called" << std::endl;
}

WebServer::WebServer(const char *m_ipAddress, const char *m_port)
	: TcpListener(m_ipAddress, m_port)
{
	std::cout << "Paramterized Constructor for WebServer called" << std::endl;
}

WebServer::WebServer(const WebServer &other)
{
	std::cout << "Copy constuctor for WebServer called" << std::endl;
	*this = other;
}

WebServer::~WebServer()
{
	std::cout << "Destructor for WebServer called" << std::endl;
}

WebServer& WebServer::operator=(const WebServer& other)
{
	std::cout << "Assignment operator for WebServer called" << std::endl;
	if (this != &other)
	{
		return *this;
	}
	return *this;
}

void WebServer::sendResponse(int &errorCode, std::string &content, const int &clientSocket, 
	const char *contType)
{
	std::ostringstream oss;
	oss << "HTTP/1.1 " << errorCode << " OK\r\n";
	oss << "Cache-Control: no-cache, private\r\n";
	oss << "Content-Type: " << contType << "\r\n";
	oss << "Content-Length: " << content.size()  << "\r\n";
	oss << "\r\n";
	oss << content;
	std::string output = oss.str();
	int size = output.size() + 1;

	int bytesOut = sendToClient(clientSocket, oss.str().c_str(), size);
	if (bytesOut < 0)
		exit (TODO);
	else
		std::cout << "sent " << bytesOut << " bytes out" << std::endl;
}

void WebServer::sendText(std::string &content, std::vector<std::string> &parsed, int &errorCode)
{
	std::string htmlFile = "/index.html";

	if (parsed.size() >= 3 && parsed[0] == "GET")
	{	
		htmlFile = parsed[1];
		if (htmlFile == "/")
			htmlFile = "/index.html";
		
		std::ifstream f("./pages/" + htmlFile);
		if (f.good())
		{
			while (std::getline(f, content, '\0'))
				;
			errorCode = 200;
		}
		f.close(); 
	}
}

void WebServer::onMessageRecieved(const int clientSocket, const char *msg, int length)
{
	(void)length;

	//Parse out the document requested
	std::istringstream iss(msg);
	std::vector<std::string> parsed
	((std::istream_iterator<std::string>(iss)), (std::istream_iterator<std::string>()));

	//TODO: check if the header contains "Connection: close"
	//Open the document ini the local files system
	std::string content = "<h1>404 Not Found</h1>";
	int errorCode = 404;

	sendText(content, parsed, errorCode);
	sendResponse(errorCode, content, clientSocket, "text/html");

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

// void WebServer::sendImage()
// {
// 	const char* imagePath = "./pages/image.jpg";
// 	std::ifstream imageFile(imagePath, std::ios::binary | std::ios::ate);
// 	if (!imageFile.is_open()){
// 		std::cerr << "Failed to open image\n";
// 		return ;
// 	}
// 	/* tellg returns position of file pointer, gives the size of the file in bytes
// 			since it was at the end of file */
// 	std::streampos fileSize = imageFile.tellg();
// 	imageFile.seekg(std::ios::beg);


// }
