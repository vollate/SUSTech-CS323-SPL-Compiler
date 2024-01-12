#pragma once
#include "asm.hpp"

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <unordered_map>

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
                                                                 "move $v0, {}\njr $ra",
                                                                 "{} {}, {}, z",
                                                                 R"(addi $sp, $sp, -4
sw $ra, 0($sp)
jal read
lw $ra, 0($sp)
addi $sp, $sp, 4
move {}, $v0)",
                                                                 R"(
move $a0, {}
addi $sp, $sp, -4
sw $ra, 0($sp)
jal write
lw $ra, 0($sp)
addi $sp, $sp, 4
    )",
                                                                 R"(jal {})",  // TODO
                                                                 R"(addi $sp, $sp, -4
sw {}, 0($sp))" };

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
    s0,
    s1,
    s2,
    s3,
    s4,
    s5,
    s6,
    s7,
    t8,
    t9,
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
    std::unordered_map<std::string_view, VarDesc<MIPS32_REG>> variables;
    std::array<RegDesc<MIPS32_REG>, 32> registers;
    uint32_t stackPtr;

    MIPS32_REG getRegister(const TacInst& op);
    MIPS32_REG getRegisterW(const TacInst& op);
    void spillRegister(std::fstream& file, MIPS32_REG reg);
    std::string regToString(MIPS32_REG reg);

public:
    virtual void reset() override;
    virtual void preTranslate(std::fstream& file) override;
    virtual void postTranslate(std::fstream& file) override;
    virtual void translateInst(std::fstream& file, const TacInst& inst) override;
    MIPS32();
};
