#include <iostream>

int main() {
  std::string line;
  std::cerr << "I'm working";
  std::cout << "Content-Type: text/plain\r\n\r\n" << std::flush;
  std::cin >> line;
  std::cout << line << std::flush;
}