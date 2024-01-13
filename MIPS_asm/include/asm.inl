#pragma once
#include "asm.hpp"
#include "utils.hpp"
#include <asm.hpp>
#include <cstdarg>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

template <typename Reg>
TacInst::TacOpd Assembler<Reg>::processVariable(const std::string& varStr) {
    if(varStr.empty()) {
        throw std::runtime_error("variable str length is 0 !!!");
    } else if(varStr[0] == '#') {
        return std::atoi(varStr.c_str() + 1);
    }
    return varStr;
}

template <typename Reg>
void TargetPlateform<Reg>::setOutPath(const std::string& outPath) {
    file = std::fstream(outPath, std::ios::out);
}

template <typename Reg>
void TargetPlateform<Reg>::setFuncInfo(std::list<FunctionInfo>* info) {
    funcInfo = info;
}

template <typename Reg>
void TargetPlateform<Reg>::reset() {
    file.close();
    funcInfo = nullptr;
}

template <typename Reg>
Assembler<Reg>::Assembler(std::string_view inPath, std::string_view outPath, TargetPlateform<Reg>& target)
    : inPath{ inPath }, outPath{ outPath }, target{ target } {}

template <typename Reg>
bool Assembler<Reg>::readFile() {
    std::ifstream inFile(inPath, std::ios::in);
    if(!inFile.is_open()) {
        debugError("Cannot open file %s\n", inPath.c_str());
        return false;
    }
    std::string buffer;
    while(std::getline(inFile, buffer)) {
        lines.push_back(buffer);
    }
    return true;
}

template <typename Reg>
void Assembler<Reg>::processLine(const std::string& line) {
    std::vector<std::string> elements;
    size_t begin = 0;
    for(size_t i = 0; i < line.length(); ++i) {
        if(line[i] == ' ') {
            elements.push_back(line.substr(begin, i - begin));
            begin = i + 1;
        }
    }
    elements.push_back(line.substr(begin));
    auto& first = elements.front();
    switch(elements.size()) {
        case 2:
            if(first == "GOTO") {
                instructions.push_back({ TacInst::GOTO, { elements[1] } });
            } else if(first == "RETURN") {
                funcInfo.back().varNum = localVariables.size();
                instructions.push_back({ TacInst::RETURN, { processVariable(elements[1]) } });
            } else if(first == "ARG") {
                localVariables.insert(elements[1]);
                instructions.push_back({ TacInst::ARG, { processVariable(elements[1]) } });
            } else if(first == "PARAM") {
                ++funcInfo.back().argc;
                localVariables.insert(elements[1]);
                instructions.push_back({ TacInst::PARAM, { processVariable(elements[1]) } });
            } else if(first == "READ") {
                localVariables.insert(elements[1]);
                instructions.push_back({ TacInst::READ, { processVariable(elements[1]) } });
            } else if(first == "WRITE") {
                localVariables.insert(elements[1]);
                instructions.push_back({ TacInst::WRITE, { processVariable(elements[1]) } });
            } else {
                debugError("Error for 2 elements process");
            }
            break;
        case 3:
            if(first == "LABEL") {
                instructions.push_back({ TacInst::LABEL, { elements[1] } });
            } else if(first == "FUNCTION") {
                localVariables.clear();
                funcInfo.emplace_back(elements[1], 0);
                instructions.push_back({ TacInst::FUNCTION, { elements[1] } });
            } else if(elements[1] == ":=") {  // a:=b
                instructions.push_back({ TacInst::ASSIGN, { processVariable(elements[0]), processVariable(elements[2]) } });
            }
            break;
        case 4:  // CALL func
            instructions.push_back({ TacInst::CALL, { processVariable(elements[0]), elements[3] } });
            break;
        case 5:  // a:=b+c
            localVariables.insert(elements[0]);
            localVariables.insert(elements[2]);
            localVariables.insert(elements[4]);
            instructions.push_back(
                { TacInst::NONE, { processVariable(elements[0]), processVariable(elements[2]), processVariable(elements[4]) } });
            switch(elements[3][0]) {
                case '+':
                    instructions.back().kind = TacInst::ADD;
                    break;
                case '-':
                    instructions.back().kind = TacInst::SUB;
                    break;
                case '*':
                    instructions.back().kind = TacInst::MUL;
                    break;
                case '/':
                    instructions.back().kind = TacInst::DIV;
                    break;
                default:
                    debugError("unknow eperator: %s", elements[3].c_str());
                    throw std::runtime_error("unknow operator");
            }
            break;
        case 6:  // IF
            localVariables.insert(elements[1]);
            localVariables.insert(elements[3]);
            if(elements[2].length() == 1) {  //>|<
                if(elements[2][0] == '<') {
                    instructions.push_back({ TacInst::IFLT, { processVariable(elements[1]), processVariable(elements[3]) } });
                } else {
                    instructions.push_back({ TacInst::IFGT, { processVariable(elements[1]), processVariable(elements[3]) } });
                }
            } else {  //<=|>=|==|!=
                if(elements[2][0] == '=') {
                    instructions.push_back({ TacInst::IFEQ, { processVariable(elements[1]), processVariable(elements[3]) } });
                } else if(elements[2][0] == '!') {
                    instructions.push_back({ TacInst::IFNE, { processVariable(elements[1]), processVariable(elements[3]) } });
                } else if(elements[2][0] == '>') {
                    instructions.push_back({ TacInst::IFGE, { processVariable(elements[1]), processVariable(elements[3]) } });
                } else {
                    instructions.push_back({ TacInst::IFLE, { processVariable(elements[1]), processVariable(elements[3]) } });
                }
            }
            instructions.back().nodes.push_back(elements[5]);
            break;
        default:
            if(line.empty()) {
                instructions.emplace_back();
            } else {
                debugError("unknow line: %s\n", line.c_str());
            }
    }
}

template <typename Reg>
void Assembler<Reg>::assembly() {
    instructions.clear();
    funcInfo.clear();
    if(!readFile()) {
        std::cerr << "Open file failed\n";
        return;
    }
    for(const auto& line : lines) {
        processLine(line);
    }
    target.reset();
    target.setOutPath(outPath);
    target.setFuncInfo(&funcInfo);
    target.preTranslate();
    for(const auto& inst : instructions) {
        target.translateInst(inst);
    }
    target.postTranslate();
}
