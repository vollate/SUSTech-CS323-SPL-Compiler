#include "asm.hpp"
#include "mips32.hpp"
#include <cstdarg>
#include <fstream>
#include <iostream>
#include <string>

static void debugInfo(const char* pattern, ...) {
#ifdef SPLC_DEBUG
    va_list args;
    va_start(args, pattern);
    vprintf(pattern, args);
    va_end(args);
#endif
}

static void debugError(const char* pattern, ...) {
#ifdef SPLC_DEBUG
    va_list args;
    va_start(args, pattern);
    vfprintf(stderr, pattern, args);
    va_end(args);
#endif
}

MIPS32_ASM::MIPS32_ASM(const std::string& inPath, const std::string& outPath) : inPath(inPath), outPath(outPath) {}

void MIPS32_ASM::assembly() {
    std::ifstream inFile(inPath, std::ios::in);
    if(!inFile.is_open()) {
        debugError("Cannot open file %s\n", inPath.c_str());
        return;
    }
    std::fstream outFile(outPath, std::ios::out);
    std::string buffer;
    while(std::getline(inFile, buffer)) {
        std::cout << buffer << '\n';
    }
}
