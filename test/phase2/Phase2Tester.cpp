#include "SplFrontage.hpp"
#include <stdexcept>
#include <string>

int main(int argc, char **argv) {
    if (argc == 1) {
        throw std::runtime_error("no argument");
    }
    spl::Frontage instance(argv[1]);
    bool succeed = instance.parse();
    if (succeed) {
        succeed = instance.semantic();
        if (!succeed) {
            std::cerr << "semantic error\n";
        }
    } else {
        std::cerr << "syntax error\n";
    }
    if (argc == 3) {
        std::cout << "writing to file: " << argv[2] << '\n';
        std::fstream file(argv[2], std::ios::out);
        if (!succeed)
            file << instance.semanticError();
    }

    return 0;
}
