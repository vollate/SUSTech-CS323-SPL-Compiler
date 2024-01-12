#include "mips32.hpp"
#include "magic_enum/magic_enum.hpp"
#include <cstdint>
#include <fstream>
#include <string_view>

std::string_view MIPS32::PREAMBLE = R"(# SPL compiler generated assembly
.data
_prmpt: .asciiz "Enter an integer: "
_eol: .asciiz "\n"
.global main
.text
)";

std::string_view MIPS32::READ_FUNC = R"(read:
li $v0, 4 la $a0, 4
la $a0, _prmpt
syscall
li $v0, 5
syscall
jr $ra)";

std::string_view MIPS32::WRITE_FUNC = R"(write:
li $v0, 1
syscall
li $v0, 4
la $a0, _eol
syscall
move $v0, $0
jr $ra)";

MIPS32::MIPS32() : stackPtr{ static_cast<uint32_t>(-1) } {}

void MIPS32::reset() {
    stackPtr = -1;
    variables.clear();
    for(auto& reg : registers) {
        reg.varName.clear();
        reg.dirty = false;
    }
}

MIPS32_REG MIPS32::getRegister(const TacInst& op) {
    // TODO
    return MIPS32_REG::NUM_REGS;
}

MIPS32_REG MIPS32::getRegisterW(const TacInst& op) {
    // TODO
    return MIPS32_REG::NUM_REGS;
}

void MIPS32::spillRegister(std::fstream& file, MIPS32_REG reg) {
    file << std::format(MIPS32_Format[15], regToString(reg)) << '\n';
    // TODO
}

void MIPS32::preTranslate(std::fstream& file) {
    file << PREAMBLE << READ_FUNC << WRITE_FUNC;
}

void MIPS32::translateInst(std::fstream& file, const TacInst& inst) {
    using TI = TacInst;
    switch(inst.kind) {  // TODO
        case TI::ASSIGN:
            break;
        case TI::ADD:
            break;
        case TI::ARG:
            break;
        case TI::ADDR:
            break;
        case TI::CALL:
            break;
        case TI::DEC:  // no requirement
            break;
        case TI::DIV:
            break;
        case TI::DEREF:
            break;
        case TI::FETCH:
            break;
        case TI::FUNCTION:
            break;
        case TI::GOTO:
            break;
        case TI::IFEQ:
            break;
        case TI::IFNE:
            break;
        case TI::IFLT:
            break;
        case TI::IFLE:
            break;
        case TI::IFGT:
            break;
        case TI::IFGE:
            break;
        case TI::LABEL:
            break;
        case TI::MUL:
            break;
        case TI::PARAM:
            break;
        case TI::RETURN:
            break;
        case TI::SUB:
            break;
        case TI::READ:
            file << std::format(MIPS32_Format[13], "todo: read");  // TODO
            break;
        case TI::WRITE:
            file << std::format(MIPS32_Format[14], "todo: write");  // TODO
            break;
        case TI::NONE:
            break;
    }
    file << '\n';
}

std::string MIPS32::regToString(MIPS32_REG reg) {
    return "$" + std::string{ magic_enum::enum_name<MIPS32_REG>(reg) };
}

void MIPS32::postTranslate(std::fstream& file){};
