#pragma once
#include <string>

class MIPS32_ASM {
    std::string inPath, outPath;

public:
    MIPS32_ASM(const std::string& inPath, const std::string& outPath);
    void assembly();
};
