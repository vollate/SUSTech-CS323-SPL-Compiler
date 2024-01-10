#pragma once
#include <cstdarg>
#include <format>
#include <fstream>
#include <list>
#include <string>
#include <variant>
#include <vector>

struct TacInst {
    using NodeType = std::variant<int32_t, std::string, TacInst>;
    enum {
        LABEL,
        FUNCTION,
        ASSIGN,
        ADD,
        SUB,
        MUL,
        DIV,
        ADDR,
        FETCH,
        DEREF,
        GOTO,
        IFLT,
        IFLE,
        IFGT,
        IFGE,
        IFNE,
        IFEQ,
        RETURN,
        DEC,
        ARG,
        CALL,
        PARAM,
        READ,
        WRITE,
        NONE
    } kind = NONE;
    std::vector<NodeType> nodes;
};

template <typename Reg>
class TargetPlateform {
public:
    virtual void preTranslate(std::fstream& file) = 0;
    virtual void translateInst(std::fstream& file, const TacInst& ins) = 0;
};

template <typename Reg>
class Assembler {
    std::string inPath, outPath;
    std::vector<std::string> lines;
    std::list<TacInst> instructions;
    TargetPlateform<Reg>& target;
    bool readFile();
    void processLine(const std::string& line);
    TacInst::NodeType processVariable(const std::string& varStr);

public:
    Assembler(const std::string& inPath, const std::string& outPath, TargetPlateform<Reg>& target);
    void assembly();
};

#include "asm.inl"
