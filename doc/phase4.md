# Phase 4

## Requirement

- cmake >= 3.14
- 支持 C++20 标准的编译器
- C++20 标准库

## Installation

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release #-DCMAKE_CXX_COMPILER=xxx 指定编译器时使用
cmake --build build
cd build && make install && cd ..
# run splc
./bin/splc
```

## 项目介绍

项目使用 C++20 标准(~~C++17 + std::format，不想再引入一个 fmt 库~~)，使用开源库 [magic_enum](https://github.com/Neargye/magic_enum) 来简化枚举到字符串的转换操作。

项目实现了寄存器的本地分配，在每一个逻辑区间分(label开始到跳转操作结束)分配 `$t0~9` 寄存器。每个变量在当前栈帧上有一块固定的内存用于写回操作。

### 分配寄存规则

- 若变量已经在寄存器中，直接返回
- 先检查是否有空闲寄存器，若有返回第一个空闲寄存器
- 若没有，对不在白名单中的寄存器进行写回操作，返回被写回的寄存器。优先写回 dirtybit=false 的寄存器，这样可以省去在汇编代码中实际生成写回的操作。
- 将当前寄存器加入到白名单

>每条指令翻译后会重置寄存器白名单

### 写回规则

- 若 dirtybit=false，仅标记写回
- 否则，生成对应汇编指令

### 函数调用

- 没有做栈传参，使用 \$a0-3 + \$s0-7 传参，最多支持 12 个参数（绝大多数情况够用了
- 由 callee 负责 \$ra, \$fp, \$sp 的存储和恢复。进入函数后，首先根据当前函数局部变量数分配新栈帧，然后存储 $ra 和 \$fp 并重新赋值 \$fp，退出时逆向操作即可。

## 项目设计

使用对象封装来避免全局变量。使用模板和继承保证项目的可拓展性。

```c++
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
```

Assembler 类负责 IR 的读取并转换为定义的 instruction 以供转换,通过 TargetPlateform 模板类的接口生成对应平台的汇编代码

```c++
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

class MIPS32 : public TargetPlateform<MIPS32_REG> {
    // codes ...
};

```

通过 TargetPlateform 模板虚基类的接口实现多态。支持不同平台的汇编代码生成，只需要继承并实现对应平台的 TargetPlateform 模板类即可。

