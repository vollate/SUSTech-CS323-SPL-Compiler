#include "mips32.hpp"
#include "magic_enum/magic_enum.hpp"

#include <format>
#include <fstream>
#include <limits>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

namespace {
    template<typename... T>
    struct overload : T ... {
        using T::operator()...;
    };

    template<typename... T>
    overload(T...) -> overload<T...>;

    std::string opdToStr(const TacInst::TacOpd &opd) {
        return std::get<std::string>(opd);
    }

    int opdToInt(const TacInst::TacOpd &opd) {
        return std::get<int>(opd);
    }

    std::string regToStr(MIPS32_REG reg) {
        return "$" + std::string{magic_enum::enum_name<MIPS32_REG>(reg)};
    }

#define getStrReg(__opd) regToStr(getRegister(__opd))
#define getStrRegW(__opd) regToStr(getRegisterW(__opd))
}  // namespace

std::string_view MIPS32::PREAMBLE = R"(# SPL compiler generated assembly
.data
_prmpt: .asciiz "Enter an integer: "
_eol: .asciiz "\n"
.globl main
.text)";

std::string_view MIPS32::READ_FUNC = R"(
read:
li $v0, 4
la $a0, _prmpt
syscall
li $v0, 5
syscall
jr $ra)";

std::string_view MIPS32::WRITE_FUNC = R"(
write:
li $v0, 1
syscall
li $v0, 4
la $a0, _eol
syscall
move $v0, $0
jr $ra)";

void MIPS32::reset() {
    TargetPlateform<MIPS32_REG>::reset();
    stackPtr = std::numeric_limits<int32_t>::max();
    regWhiteList.clear();
    curFunc = nullptr;
    curFrameOffset = curSpOffset = 0;
    paramCounter = argCounter = 0;
    variables.clear();
    for (auto &reg: registers) {
        reg.varName.clear();
        reg.dirty = true;
    }
}

bool MIPS32::inWhiteList(MIPS32_REG reg) {
    return std::find(regWhiteList.begin(), regWhiteList.end(), reg) != regWhiteList.end();
}

MIPS32_REG MIPS32::getFreeRegister() {
    std::list<int> cleanReg;
    for (int i = static_cast<int>(MIPS32_REG::t0); i <= static_cast<int>(MIPS32_REG::t9); ++i) {
        if (!registers[i].dirty) {
            cleanReg.push_back(i);
        }
        if (registers[i].varName.empty() && !inWhiteList(static_cast<MIPS32_REG>(i))) {
            return static_cast<MIPS32_REG>(i);
        }
    }
    for (int i: cleanReg) {
        MIPS32_REG reg = static_cast<MIPS32_REG>(i);
        if (!inWhiteList(reg)) {
            spillRegister(reg);
            return reg;
        }
    }
    MIPS32_REG res;
    for (int i = static_cast<int>(MIPS32_REG::t0); i <= static_cast<int>(MIPS32_REG::t9); ++i) {
        MIPS32_REG reg = static_cast<MIPS32_REG>(i);
        if (!inWhiteList(reg)) {
            spillRegister(reg);
            res = reg;
            break;
        }
    }
    return res;
}

MIPS32_REG MIPS32::getRegister(const TacInst::TacOpd &op) {
    MIPS32_REG res = MIPS32_REG::zero;
    auto varName = std::get<std::string>(op);
    if (variables.count(varName) == 0) {
        res = getFreeRegister();
        curSpOffset -= 4;
        registers[static_cast<int>(res)] = {varName, true};
#ifdef SPLC_DEBUG
        file << std::format("# new var {}, store in reg {}\n", varName, regToStr(res));
#endif
        variables.insert({varName, {varName, res, curSpOffset + stackPtr}});
    } else {
        auto &varDesc = variables[varName];
        if (registers[static_cast<int>(varDesc.reg)].varName == varName) {
            res = varDesc.reg;
        } else {
            res = getFreeRegister();
            file << std::format(MIPS32_Format[7], regToStr(res), varDesc.pos - stackPtr, "$sp")
                 #ifdef SPLC_DEBUG
                 << std::format(" # load {} to reg {}\n", varDesc.varName, regToStr(res));
#else
            << '\n';
#endif
            registers[static_cast<int>(res)] = {varName, false};
        }
    }
    regWhiteList.push_back(res);
    return res;
}

MIPS32_REG MIPS32::getRegisterW(const TacInst::TacOpd &op) {
    // MIPS32_REG res = MIPS32_REG::zero;
    auto res = getRegister(op);
    registers[static_cast<int>(res)].dirty = true;
    return res;
}

void MIPS32::spillRegister(MIPS32_REG reg) {
    auto &regDesc = registers[static_cast<int>(reg)];
    if (regDesc.dirty && !regDesc.varName.empty()) {
        auto &varDesc = variables[regDesc.varName];
        regDesc.dirty = false;
        file << std::format(MIPS32_Format[17], regToStr(reg), varDesc.pos - stackPtr)
             #ifdef SPLC_DEBUG
             << std::format(" # spill {} from reg {}\n", varDesc.varName, regToStr(reg));
#else
        << '\n';
#endif
    }
}

void MIPS32::spillAllRegs() {
    for (int i = static_cast<int>(MIPS32_REG::t0); i <= static_cast<int>(MIPS32_REG::t9); ++i) {
        spillRegister(static_cast<MIPS32_REG>(i));
        registers[i].varName.clear();
    }
}

std::tuple<std::string, std::string, std::string> MIPS32::processASMDVars(const TacInst &inst) {
    std::string second, third;
    std::string first = getStrRegW(inst.nodes[0]);
    if (inst.nodes[1].index() == inst.nodes[2].index()) {
        if (0 == inst.nodes[1].index()) {
            auto reg1 = getFreeRegister();
            auto reg2 = getFreeRegister();
            second = regToStr(reg1);
            third = regToStr(reg2);
            file << std::format(MIPS32_Format[0], second, opdToInt(inst.nodes[1])) << '\n'
                 << std::format(MIPS32_Format[0], third, opdToInt(inst.nodes[2])) << '\n';
        } else {
            second = getStrReg(inst.nodes[1]);
            third = getStrReg(inst.nodes[2]);
        }
    } else {
        if (0 == inst.nodes[1].index()) {
            third = getStrReg(inst.nodes[2]);
            auto reg1 = getFreeRegister();
            second = regToStr(reg1);
            file << std::format(MIPS32_Format[0], second, opdToInt(inst.nodes[1])) << '\n';
        } else {
            second = getStrReg(inst.nodes[1]);
            auto reg2 = getFreeRegister();
            third = regToStr(reg2);
            file << std::format(MIPS32_Format[0], third, opdToInt(inst.nodes[2])) << '\n';
        }
    }
    return {first, second, third};
}

void MIPS32::preTranslate() {
    file << PREAMBLE << '\n' << READ_FUNC << '\n' << WRITE_FUNC << '\n';
}

void MIPS32::translateInst(const TacInst &inst) {
    using TI = TacInst;
    const static auto emitIF = [this](std::string_view compare, const TacInst &inst) {
        spillAllRegs();
        auto &l = inst.nodes[0], &r = inst.nodes[1];
        if (l.index() == r.index()) {
            if (l.index() == 0) {
                int lVal = std::get<int>(l);
                int rVal = std::get<int>(r);
                file << std::format(MIPS32_Format[12], compare, lVal, rVal, std::get<std::string>(inst.nodes[2]));
            } else {
                std::string lVal = getStrReg(l);
                std::string rVal = getStrReg(r);
                file << std::format(MIPS32_Format[12], compare, lVal, rVal, std::get<std::string>(inst.nodes[2]));
            }
        } else {
            if (l.index() == 0) {
                file << std::format(MIPS32_Format[12], compare, std::get<int>(l), getStrReg(r),
                                    std::get<std::string>(inst.nodes[2]));
            } else {
                file << std::format(MIPS32_Format[12], compare, getStrReg(l), std::get<int>(r),
                                    std::get<std::string>(inst.nodes[2]));
            }
        }
    };

    switch (inst.kind) {
        case TI::ASSIGN:
            if (0 == inst.nodes[1].index()) {
                file << std::format(MIPS32_Format[0], getStrRegW(inst.nodes[0]), std::get<int32_t>(inst.nodes[1]));
            } else {
                file << std::format(MIPS32_Format[1], getStrRegW(inst.nodes[0]), getStrReg(inst.nodes[1]));
            }
            break;
        case TI::ADD: {
            auto [first, second, third] = processASMDVars(inst);
            file << std::format(MIPS32_Format[3], first, second, third);
            break;
        }
        case TI::ARG:
            if (argCounter++ < 4) {
                file << std::format(MIPS32_Format[1], "$a" + std::to_string(argCounter - 1), getStrReg(inst.nodes[0]));
            } else if (argCounter <= 12) {
                file << std::format(MIPS32_Format[1], "$s" + std::to_string(argCounter - 5), getStrReg(inst.nodes[0]));
            } else {
                debugError("Args more than 12 !!!\n");
            }
            break;
        case TI::ADDR:
            file << std::format(MIPS32_Format[1], getStrRegW(inst.nodes[0]), getStrReg(inst.nodes[1]));
            break;
        case TI::CALL:
            spillAllRegs();
            argCounter = 0;
            file << std::format(MIPS32_Format[10], opdToStr(inst.nodes[1]), getStrRegW(inst.nodes[0]));
            break;
        case TI::DEC:  // no requirement
            std::cerr << "DEC is not implement";
            break;
        case TI::DIV: {
            auto [first, second, third] = processASMDVars(inst);
            file << std::format(MIPS32_Format[6], second, third, first);
            break;
        }
        case TI::DEREF:
            file << std::format(MIPS32_Format[8], getStrRegW(inst.nodes[0]), 0, getStrReg(inst.nodes[1]));
            break;
        case TI::FETCH:
            file << std::format(MIPS32_Format[7], getStrRegW(inst.nodes[0]), 0, getStrReg(inst.nodes[1]));
            break;
        case TI::FUNCTION: {
            auto funName = opdToStr(inst.nodes[0]);
            if (auto res = std::find_if(funcInfo->begin(), funcInfo->end(), [&](auto &i) { return i.name == funName; });
                    res != funcInfo->end()) {
                curFunc = &*res;
                curFrameOffset = curSpOffset = 8 + curFunc->varNum * 4;
                file << std::format(MIPS32_Format[16], funName, -curSpOffset, curSpOffset - 4, curSpOffset - 8);
                stackPtr -= curFrameOffset;
                curSpOffset -= 8;
            } else {
                debugError("Function name %s not find in info table", funName.c_str());
            }
        }
            break;
        case TI::GOTO:
            spillAllRegs();
            file << std::format(MIPS32_Format[9], opdToStr(inst.nodes[0]));
            break;
        case TI::IFEQ:
            emitIF("beq", inst);
            break;
        case TI::IFNE:
            emitIF("bne", inst);
            break;
        case TI::IFLT:
            emitIF("blt", inst);
            break;
        case TI::IFLE:
            emitIF("ble", inst);
            break;
        case TI::IFGT:
            emitIF("bgt", inst);
            break;
        case TI::IFGE:
            emitIF("bge", inst);
            break;
        case TI::LABEL:
            file << opdToStr(inst.nodes[0]) << ':';
            break;
        case TI::MUL: {
            auto [first, second, third] = processASMDVars(inst);
            file << std::format(MIPS32_Format[5], first, second, third);
            break;
        }
        case TI::PARAM:
            if (curFunc->argc <= 4) {
                ++paramCounter;
                file << std::format(MIPS32_Format[1], getStrReg(inst.nodes[0]),
                                    "$a" + std::to_string(4 - paramCounter));
            } else if (curFunc->argc <= 12) {
                if (paramCounter++ < curFunc->argc - 4) {
                    file << std::format(MIPS32_Format[1], getStrReg(inst.nodes[0]),
                                        "$s" + std::to_string(curFunc->argc - 4 - paramCounter));
                } else {
                    file << std::format(MIPS32_Format[1], getStrReg(inst.nodes[0]),
                                        "$a" + std::to_string(curFunc->argc - paramCounter));
                }
            } else {
                debugError("Paramters more than 12 !!!\n");
            }
            break;
        case TI::RETURN:
            paramCounter = 0;
            file << std::format(MIPS32_Format[11], curFrameOffset - 4, curFrameOffset - 8, curFrameOffset,
                                getStrReg(inst.nodes[0]));
            break;
        case TI::SUB: {
            auto [first, second, third] = processASMDVars(inst);
            file << std::format(MIPS32_Format[4], first, second, third);
            break;
        }
            break;
        case TI::READ:
            file << std::format(MIPS32_Format[13], getStrRegW(inst.nodes[0]));
            break;
        case TI::WRITE:
            file << std::format(MIPS32_Format[14], getStrReg(inst.nodes[0]));
            break;
        case TI::NONE:
            break;
    }
    file << '\n';
    regWhiteList.clear();
}

void MIPS32::postTranslate() {};
