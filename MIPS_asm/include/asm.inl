#pragma once
#include "asm.hpp"
#include "utils.hpp"
#include <asm.hpp>
#include <cstdarg>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

template <typename Reg>
TacInst::NodeType Assembler<Reg>::processVariable(const std::string& varStr) {
    if(varStr.empty()) {
        throw std::runtime_error("variable str length is 0 !!!");
    } else if(varStr[0] == '#') {
        return std::atoi(varStr.c_str() + 1);
    }
    return varStr;
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
                instructions.push_back({ TacInst::RETURN, { processVariable(elements[1]) } });
            } else if(first == "ARG") {
                instructions.push_back({ TacInst::ARG, { processVariable(elements[1]) } });
            } else if(first == "PARAM") {
                instructions.push_back({ TacInst::PARAM, { processVariable(elements[1]) } });
            } else if(first == "READ") {
                instructions.push_back({ TacInst::READ, { processVariable(elements[1]) } });
            } else if(first == "WRITE") {
                instructions.push_back({ TacInst::WRITE, { processVariable(elements[1]) } });
            } else {
                debugError("Error for 2 elements process");
            }
            break;
        case 3:
            if(first == "LABEL") {
                instructions.push_back({ TacInst::WRITE, { elements[1] } });
            } else if(first == "FUNCTION") {
                instructions.push_back({ TacInst::WRITE, { elements[1] } });
            } else if(elements[1] == ":=") {  // a:=b
                instructions.push_back({ TacInst::ASSIGN, { processVariable(elements[0]), processVariable(elements[2]) } });
            }
            break;
        case 4:  // CALL func
            instructions.push_back({ TacInst::CALL, { processVariable(elements[0]), elements[3] } });
            break;
        case 5:  // a:=b+c
            instructions.push_back({ TacInst::ASSIGN, { processVariable(elements[0]) } });
            switch(elements[3][0]) {
                case '+':
                    instructions.back().nodes.push_back(
                        TacInst{ TacInst::ADD, { processVariable(elements[2]), processVariable(elements[4]) } });
                    break;
                case '-':
                    instructions.back().nodes.push_back(
                        TacInst{ TacInst::SUB, { processVariable(elements[2]), processVariable(elements[4]) } });
                    break;
                case '*':
                    instructions.back().nodes.push_back(
                        TacInst{ TacInst::MUL, { processVariable(elements[2]), processVariable(elements[4]) } });
                    break;
                case '/':
                    instructions.back().nodes.push_back(
                        TacInst{ TacInst::DIV, { processVariable(elements[2]), processVariable(elements[4]) } });
                    break;
                default:
                    debugError("unknow eperator: %s", elements[3].c_str());
                    throw std::runtime_error("unknow operator");
            }
            break;
        case 6:                              // IF
            if(elements[2].length() == 1) {  //>|<
                if(elements[2][0] == '>') {
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
    if(!readFile()) {
        std::cerr << "Open file failed\n";
        return;
    }
    for(const auto& line : lines) {
        processLine(line);
    }
    std::fstream outFile(outPath, std::ios::out);
    target.reset();
    target.preTranslate(outFile);
    for(const auto& inst : instructions) {
        target.translateInst(outFile, inst);
    }
    target.postTranslate(outFile);
}
