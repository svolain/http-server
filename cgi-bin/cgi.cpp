#include <iostream>
#include <fstream>

int main() {
  std::cout << "Content-Type: text/plain\r\n\r\n" << std::flush;
  // std::cout << "Hello world!\r\n" << std::flush;

  std::string line;
  std::ifstream file("/home/dshatilo/projects/github/12_webserv/cgi-bin/infile_big.txt");


  while (std::getline(file, line))
    std::cout << line << std::endl;
}