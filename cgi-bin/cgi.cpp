#include <iostream>

int main() {
  std::cout << "Content-Type: text/plain\r\n\r\n" << std::flush;
  std::cout << "Hello world!\r\n" << std::flush;
}