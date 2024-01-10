#include "mips32.hpp"
#include <string>

std::string MIPS32::PREAMBLE = R"(# SPL compiler generated assembly
.data
_prmpt: .asciiz "Enter an integer: "
_eol: .asciiz "\n"
.global main
.text
)";

std::string MIPS32::READ_FUNC = R"(read:
li $v0, 4 la $a0, 4
la $a0, _prmpt
syscall
li $v0, 5
syscall
jr $ra)";
std::string MIPS32::WRITE_FUNC = R"(write:
li $v0, 1
syscall
li $v0, 4
la $a0, _eol
syscall
move $v0, $0
jr $ra)";

MIPS32::MIPS32() {}

MIPS_REG MIPS32::getRegister(const TacInst& op) {
    // TODO
    return MIPS_REG::NUM_REGS;
}

MIPS_REG MIPS32::getRegisterW(const TacInst& op) {
    // TODO
    return MIPS_REG::NUM_REGS;
}

void MIPS32::spillRegister(MIPS_REG reg) {
    // TODO
}

void MIPS32::preTranslate(std::fstream& file) {
    file << PREAMBLE << READ_FUNC << WRITE_FUNC;
}

void MIPS32::translateInst(std::fstream& file, const TacInst& inst) {
    using TI = TacInst;
    switch(inst.kind) {  // TODO
        case TI::ASSIGN:
            return;
        case TI::ADD:
            return;
        case TI::ARG:
            return;
        case TI::ADDR:
            return;
        case TI::CALL:
            return;
        case TI::DEC:
            return;
        case TI::DIV:
            return;
        case TI::DEREF:
            return;
        case TI::FETCH:
            return;
        case TI::FUNCTION:
            return;
        case TI::GOTO:
            return;
        case TI::IFEQ:
            return;
        case TI::IFNE:
            return;
        case TI::IFLT:
            return;
        case TI::IFLE:
            return;
        case TI::IFGT:
            return;
        case TI::IFGE:
            return;
        case TI::LABEL:
            return;
        case TI::MUL:
            return;
        case TI::PARAM:
            return;
        case TI::RETURN:
            return;
        case TI::SUB:
            return;
        case TI::NONE:
            file << '\n';
    }
}
