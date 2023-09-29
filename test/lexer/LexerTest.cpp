#include "Lexer.hpp"
#include <stdexcept>
#include <string>

int main(int argc, char **argv) {
  if (argc == 1) {
    throw std::runtime_error("no argument");
  }
    Lexer instance;
  for (int i = 1; i < argc; ++i) {
    std::cout << instance.lexicalAnalysis(argv[i]) << std::endl;
  }

  return 0;
}
