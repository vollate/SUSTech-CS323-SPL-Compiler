#include "asm.hpp"
#include <iostream>

int main(int argc, char** argv) {
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }
    std::string inPath(argv[1]);
    auto outPath = inPath.substr(0, inPath.find_last_of('.')) + ".s";
    MIPS32_ASM masm(inPath, outPath);
    masm.assembly();
}
