# SUSTech CS323 Compiler Project

本 project 使用 flex + bison 的 C++ 模式 (Bison 和 Flex 对 C++ 的支持依托)，并进行了模块化处理，使得可以同时启用多个实例进行 parse + sematic analyze。~~想了想没用，现在C/CPP编译器都是单进程的，但是用对象封装至少能保证没全局变量的屎~~

全程使用智能指针避免内存泄漏，使用类型安全的 `variant` 代替不安全的 union, 数据结构部分使用了 `any`

只做到 phase2, 之后由于时间原因转而使用队友的 C 框架

由于敏捷开发，数据结构设计有很多不合理的地方并且懒得重构了。
