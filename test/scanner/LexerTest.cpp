#include <stdexcept>
#include <string>

#include "SplFrontage.hpp"

int main(int argc, char **argv) {
    if (argc == 1) {
        throw std::runtime_error("no argument");
    }
    spl::Frontage instance;
    for (int i = 1; i < argc; ++i) {
//        std::cout << instance.parse(argv[i]) << std::endl;
    }

    return 0;
}
