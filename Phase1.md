# Phase1

项目采用Flex和Bison的C++模式，使用智能指针用来规避C的复杂内存管理，方便使用字符串，并且语法灵活性更高。而且使用预编译头文件，提高编译速度，简化依赖管理。

### Requirement

- Bison 3.8+(to support variant)
- cmake 3.10+
- flex

### 运行

执行以下命令:

```bash
cmake -B build
cd build&&cmake --build .
make install
```
可以在build文件夹下执行ctest,会输出结果到
### Basic

##### Scanner

Scanner部分，实现对多种进制整数的匹配，还可以判别十六进制整数的正确性。实现忽略单行注释（以 // 开始）。实现忽略多行注释（以 /\* 开始并以 \*/ 结束）。

```c++
digit_hex [0-9a-fA-F]
invalid_hex_char [g-zG-Z]
int32_dec (0|([1-9]{digit}*))
int32_hex 0[xX]({digit_hex}+)
int32_hex_illgeal_char 0[xX]({digit_hex}*{invalid_hex_char}+{digit_hex}*)
```

使用宏定义实现DEBUG模式下，它将打印令牌的类型和值；否则，它只返回该令牌。方便调试。

```
#ifdef SPL_DEBUG
#define GENERATE_RETURN_TOKEN(__name,__type,__value)\
{ cout<<"type: "<<#__name<<"\tvalue: "<<__value<<'\n';\
return Parser::make_##__name(std::make_unique<ASTNode>(token_type::__type,m_frontage.location(),__value),m_frontage.location());}
#else
    #define GENERATE_RETURN_TOKEN(__name,__type,__value)\
    return Parser::make_##__name(std::make_unique<ASTNode>(token_type::__type,m_frontage.location(),__value),m_frontage.location())
#endif
```

##### Parser

我们使用ASTNode结构用来存储语法树的节点，每个节点有对应类型，参数和对应的位置信息以及子节点，构造函数使用右值引用和转发，对象赋值可以避免无用拷贝。
定义spl命名空间确保不会产生命名冲突。

```c++
namespace spl {
        class Scanner;
        class Frontage;
        class Parser;
        struct ASTNode {
            using variant_type = std::variant<int32_t, float, std::string>;
            int32_t type;
            variant_type value;
            location loc;
            std::list<std::unique_ptr<ASTNode>> subNodes;
            ASTNode(int32_t type,location&& loc, variant_type &&value) : type(type),loc(std::forward<spl::location>(loc)), value(std::forward<variant_type>(value)) {
            #ifdef SPL_DEBUG
            cout<<"new node location: "<<loc<<'\n';
            #endif
            }
        };
    }
    using NodeType = std::unique_ptr<spl::ASTNode>;
```

定义NodeType作为类型别名，可以通过更简单的方式引用。使用智能指针确保该节点的唯一指定，并且防止内存泄漏。

```c++
using NodeType = std::unique_ptr<spl::ASTNode>;
```

我们引入yyerror处理错误信息，并增加枚举参数标识错误类型便于处理。

```c++
enum class ERROR_TYPE {
        LEXICAL_ERROR,
        SYNTAX_ERROR,
        SEMANTIC_ERROR,
        OTHER_ERROR
};
void yyerror(const char* msg, ERROR_TYPE err);
```

使用宏定义更快捷地创建ASTNode，并且唯一指针。

```c++
#define BUILD_AST_NODE(__type,__value)\
std::make_unique<ASTNode>(token_type::__type,frontage.location(), __value)
```

在Token定义部分启用location，用于错误跟踪，方便输出错误位置

```
%locations
```

### Bonus

● 实现识别单行注释

● 实现识别多行注释

