# Phase 3

## IR optimize

### Unreachable code elimination

1. 判断块前是否有无条件跳转的 `GOTO` 指令，如果有则不可达。不可达块中出现 `LABEL` 则认为该不可达块结束，后面的块恢复为可达。
2. `IF` 指令跳转如果恒为真，则检查跳转的label,如果未定义，则后方的块都不可达知道找到label定义，然后删除 `IF` 和对应label。否则不处理。（数据流分析好麻烦，摆了）

### Constant folding

同上，数据流分析摆了

>写的一坨屎，而且有内存泄漏，写 c 没有 RAII 好麻烦

