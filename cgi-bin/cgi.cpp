#include <iostream>

int main() {
  std::string line;
  std::cout << "Content-Type:text/html\r\n\r\n" << std::flush;
  std::cin >> line;
  line = line.substr(line.find('=') + 1);
  std::cout << "<!doctype html><html><head><title>cgi message</title></head><body><p>Hello "
            << line <<"!</p></body></html>" << std::flush;
}