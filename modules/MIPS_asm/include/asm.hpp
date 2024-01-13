#pragma once
#include <cstdarg>
#include <cstdint>
#include <format>
#include <fstream>
#include <list>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

struct TacInst {
    using TacOpd = std::variant<int32_t, std::string>;
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
    std::vector<TacOpd> nodes;
};

struct FunctionInfo {
    std::string name;
    int argc = 0;
    int varNum = 0;
};

template <typename Reg>
struct VarDesc {
    std::string varName;
    Reg reg;
    int32_t pos;
};

template <typename Reg>
struct RegDesc {
    std::string varName;
    bool dirty = false;
};

template <typename Reg>
class TargetPlateform {
protected:
    std::fstream file;
    std::list<FunctionInfo>* funcInfo;

public:
    void setOutPath(const std::string& filePath);
    void setFuncInfo(std::list<FunctionInfo>* info);
    virtual void reset();
    virtual void preTranslate() = 0;
    virtual void postTranslate() = 0;
    virtual void translateInst(const TacInst& ins) = 0;
};

template <typename Reg>
class Assembler {
    std::string inPath, outPath;
    std::vector<std::string> lines;
    std::list<TacInst> instructions;
    std::set<std::string> localVariables;
    std::list<FunctionInfo> funcInfo;
    TargetPlateform<Reg>& target;
    bool readFile();
    void processLine(const std::string& line);
    TacInst::TacOpd processVariable(const std::string& varStr);

public:
    Assembler(std::string_view inPath, std::string_view outPath, TargetPlateform<Reg>& target);
    void assembly();
};

#include "asm.inl"
