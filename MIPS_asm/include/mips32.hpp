#pragma once
#include "asm.hpp"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <initializer_list>
#include <string>
#include <tuple>
#include <unordered_map>

/*
 *| 0 li
 *| 1 move
 *| 2 addi
 *| 3 add
 *| 4 sub
 *| 5 mul
 *| 6 div
 *| 7 lw
 *| 8 sw
 *| 9 jr
 *| 10 jal
 *| 11 return
 *| 12 bxx reg1 reg2 label
 *| 13 read
 *| 14 write
 *| 15 call
 *| 16 function define
 *| 17 save reg to stack
 */
constexpr inline std::array<std::string_view, 20> MIPS32_Format{ "li {}, {}",
                                                                 "move {}, {}",
                                                                 "addi {}, {}, {}",
                                                                 "add {}, {}, {}",
                                                                 "sub {}, {}, {}",
                                                                 "mul {}, {}, {}",
                                                                 "div {}, {}\nmflo {}",
                                                                 "lw {}, {}({})",
                                                                 "sw {}, {}({})",
                                                                 "j {}",
                                                                 "jal {}\nmove {}, $v0",
                                                                 R"(move $sp, $fp
lw $ra, {}($sp)
lw $fp, {}($sp)
addi $sp, $sp, {}
move $v0, {}
jr $ra)",
                                                                 "{} {}, {}, {}",
                                                                 R"(addi $sp, $sp, -4
sw $ra, 0($sp)
jal read
lw $ra, 0($sp)
addi $sp, $sp, 4
move {}, $v0)",
                                                                 R"(move $a0, {}
addi $sp, $sp, -4
sw $ra, 0($sp)
jal write
lw $ra, 0($sp)
addi $sp, $sp, 4)",
                                                                 "jal {}",
                                                                 R"(
{}:
addi $sp, $sp, {}
sw $ra, {}($sp)
sw $fp, {}($sp)
move $fp, $sp)",
                                                                 R"(sw {}, {}($sp))" };

enum class MIPS32_REG {
    zero,
    at,
    v0,
    v1,
    a0,
    a1,
    a2,
    a3,
    t0,
    t1,
    t2,
    t3,
    t4,
    t5,
    t6,
    t7,
    t8,
    t9,
    s0,
    s1,
    s2,
    s3,
    s4,
    s5,
    s6,
    s7,
    k0,
    k1,
    gp,
    sp,
    fp,
    ra,
    NUM_REGS
};

class MIPS32 : public TargetPlateform<MIPS32_REG> {
    static std::string_view PREAMBLE, READ_FUNC, WRITE_FUNC;
    std::unordered_map<std::string, VarDesc<MIPS32_REG>> variables;
    std::array<RegDesc<MIPS32_REG>, 32> registers;
    std::list<MIPS32_REG> regWhiteList;
    int32_t stackPtr, curFrameOffset, curSpOffset;
    int32_t paramCounter, argCounter;
    FunctionInfo* curFunc;
    bool inWhiteList(MIPS32_REG reg);
    std::tuple<std::string, std::string, std::string> processASMDVars(const TacInst& inst);
    MIPS32_REG getFreeRegister();
    MIPS32_REG getRegister(const TacInst::TacOpd& op);
    MIPS32_REG getRegister(std::string_view op);
    MIPS32_REG getRegisterW(const TacInst::TacOpd& op);
    void spillRegister(MIPS32_REG reg);
    void spillAllRegs();
    // std::string translateSubInst(const TacInst& inst);

public:
    virtual void reset() override;
    virtual void preTranslate() override;
    virtual void postTranslate() override;
    virtual void translateInst(const TacInst& inst) override;
    MIPS32() = default;
};
