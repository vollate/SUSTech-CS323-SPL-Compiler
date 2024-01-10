#pragma once
#include <asm.hpp>
#include <string>

const inline std::array<std::string, 20> MIPS_Format{ "li ${}, {}",
                                                      "move ${}, ${}",
                                                      "addi ${}, ${}, {}",
                                                      "add ${}, ${}, ${}",
                                                      "sub ${}, ${}, ${}",
                                                      "mul ${}, ${}, ${}",
                                                      "div ${}, ${}\nmflo ${}",
                                                      "lw ${}, 0(${})",
                                                      "sw ${},0(${})",
                                                      "j {}",
                                                      "jal {}\nmove ${}, $v0",
                                                      "move $v0, ${}\njr $ra",
                                                      "{} ${}, ${}, z" };

enum class MIPS_REG {
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

class MIPS32 : public TargetPlateform<MIPS_REG> {
    static std::string PREAMBLE, READ_FUNC, WRITE_FUNC;

    MIPS_REG
    getRegister(const TacInst& op);
    MIPS_REG getRegisterW(const TacInst& op);
    void spillRegister(MIPS_REG reg);

public:
    virtual void preTranslate(std::fstream& file) override;
    virtual void translateInst(std::fstream& file, const TacInst& inst) override;
    MIPS32();
};
