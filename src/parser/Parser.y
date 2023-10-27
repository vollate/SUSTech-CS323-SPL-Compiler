%language "c++"
%require "3.0"
%defines
%define api.parser.class { Parser }
%define api.token.constructor
%define api.value.type variant
%define parse.assert
%define api.namespace { spl }

%code requires
{
    #include <iostream>
    #include <string>
    #include <vector>
    #include <memory>
    #include <cstdint>
    #include <list>
    #include <variant>

    using namespace std;

    namespace spl {
        class Scanner;
        class Frontage;
        class Parser;
        struct ASTNode {
            using variant_type = std::variant<int32_t, float, std::string>;

            int32_t type;
            variant_type value;
            std::list<std::unique_ptr<ASTNode>> subNodes;

            ASTNode(int32_t type, variant_type &value) : type(type), value(std::forward<variant_type>(value)) {}
            ASTNode(int32_t type, variant_type &&value) : type(type), value(std::forward<variant_type>(value)) {}

            explicit ASTNode(int32_t type) : ASTNode(type, 0) {}
        };
    }
    using NodeType = std::unique_ptr<spl::ASTNode>;
}

%code top
{
    #include "Scanner.hpp"
    #include "Parser.hpp"
    #include "Frontage.hpp"
    #include "location.hh"

    static spl::Parser::symbol_type yylex(spl::Scanner &scanner, spl::Frontage &frontage) {
        return scanner.next_token();
    }
    // you can accomplish the same thing by inlining the code using preprocessor
    // x and y are same as in above static function
    // #define yylex(x, y) scanner.get_next_token()
    
    using namespace spl;
    using token_type = Parser::token_type;
    #define BUILD_AST_NODE(__type,__value)\
    	std::make_unique<ASTNode>(token_type::__type, __value)
}

%lex-param { spl::Scanner &scanner }
%lex-param { spl::Frontage &frontage }
%parse-param { spl::Scanner &scanner }
%parse-param { spl::Frontage &frontage }
%locations
%define parse.trace
%define parse.error verbose

%token END 0 "end of file"
%token <NodeType> INT;
%token <NodeType> FLOAT;
%token <NodeType> CHAR;
%token <NodeType> ID;
%token <NodeType> TYPE;
%token <NodeType> STRUCT ;
%token <NodeType> IF;
%token <NodeType> ELSE;
%token <NodeType> WHILE;
%token <NodeType> RETURN;
%token <NodeType> DOT;
%token <NodeType> SEMI;
%token <NodeType> COMMA;
%token <NodeType> ASSIGN;
%token <NodeType> LT;
%token <NodeType> LE;
%token <NodeType> GT;
%token <NodeType> GE;
%token <NodeType> NE;
%token <NodeType> EQ;
%token <NodeType> PLUS;
%token <NodeType> MINUS;
%token <NodeType> MUL;
%token <NodeType> DIV;
%token <NodeType> AND;
%token <NodeType> OR;
%token <NodeType> NOT;
%token <NodeType> LP;
%token <NodeType> RP;
%token <NodeType> LB;
%token <NodeType> RB;
%token <NodeType> LC;
%token <NodeType> RC;
%token <NodeType> ERROR;

%nonassoc <NodeType> LEXICAL_ERROR;

//!!! just used to pass value, do not use this in deduction
%nonassoc <std::string> NON_TERMINAL

%type <NodeType> Program ExtDef ExtDefList;
%%

Program: INT FLOAT {$$=BUILD_AST_NODE(NON_TERMINAL, "Program"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));frontage.m_ast.push_back(std::move($$));
/* TODO 详情如下 @YYK
 * 这条推断仅供示范，写的时候删掉这个推导,类似 https://github.com/Certseeds/CS323_Compilers_2020F/blob/master/project1/src/syntax.y
 * 但是记得加std::move，不然会报错，因为unique_ptr无拷贝构造函数
 * 待实现功能：基本推导，语法错误，词法错误
*/}
    | ExtDefList {$$=BUILD_AST_NODE(NON_TERMINAL, "Program"); $$->subNodes.push_back(std::move($1));frontage.m_ast.push_back(std::move($$));}
    | /* empty */

ExtDefList: ExtDef ExtDefList {$$=BUILD_AST_NODE(NON_TERMINAL, "ExtDefList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | /* empty */ ;

ExtDef: Specifier ExtDecList SEMI
    | Specifier SEMI
    | Specifier FunDec CompSt;

ExtDecList: VarDec
    | VarDec COMMA ExtDecList;

Specifier: TYPE
    | StructSpecifier;

StructSpecifier: STRUCT ID LC DefList RC
    | STRUCT ID;

VarDec: ID
    | VarDec LB INT RB;

FunDec: ID LP VarList RP
    | ID LP RP;

VarList: ParamDec COMMA VarList
    | ParamDec;

ParamDec: Specifier VarDec;

CompSt: LC DefList StmtList RC;

StmtList: Stmt StmtList
    | /* empty */ ;

Stmt: Exp SEMI
    | CompSt
    | RETURN Exp SEMI
    | IF LP Exp RP Stmt
    | IF LP Exp RP Stmt ELSE Stmt
    | WHILE LP Exp RP Stmt;

DefList: Def DefList
    | /* empty */ ;

Def: Specifier DecList SEMI;

DecList: Dec
    | Dec COMMA DecList;

Dec: VarDec
    | VarDec ASSIGN Exp;

Exp: Exp ASSIGN Exp
    | Exp AND Exp
    | Exp OR Exp
    | Exp LT Exp
    | Exp LE Exp
    | Exp GT Exp
    | Exp GE Exp
    | Exp NE Exp
    | Exp EQ Exp
    | Exp PLUS Exp
    | Exp MINUS Exp
    | Exp MUL Exp
    | Exp DIV Exp
    | LP Exp RP
    | MINUS Exp
    | NOT Exp
    | ID LP Args RP
    | ID LP RP
    | Exp LB Exp RB
    | Exp DOT ID
    | ID
    | INT
    | FLOAT
    | CHAR;

Args: Exp COMMA Args
    | Exp;

%%
void spl::Parser::error(const location &loc , const std::string &message) {
        cout << "Error: " << message << "\nError location: " << frontage.location() << '\n';
}
//TODO: 加入词法，语义错误的处理，显示行号等等信息 @JYF
