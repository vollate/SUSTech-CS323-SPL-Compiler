#include <format>

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
