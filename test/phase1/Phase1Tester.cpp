#include "SplFrontage.hpp"
#include <stdexcept>
#include <string>

int main(int argc, char** argv) {
  if (argc == 1) {
    throw std::runtime_error("no argument");
  }
  spl::Frontage instance(argv[1]);
  for (int i = 1; i < argc; ++i) {
    instance.parse();
  }

  return 0;
}
