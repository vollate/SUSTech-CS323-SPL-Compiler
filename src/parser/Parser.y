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

    enum error_type {
        LEXICAL_ERROR,
        SYNTAX_ERROR,
        SEMANTIC_ERROR,
        OTHER_ERROR
    };

    void yyerror(const char* msg, error_type err);

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
%nonassoc <NodeType> SYNTAX_ERROR;

//!!! just used to pass value, do not use this in deduction
%nonassoc <std::string> NON_TERMINAL

%type <NodeType> Program ExtDef ExtDefList;
%%

Program: 
/* TODO 详情如下 @YYK
 * 这条推断仅供示范，写的时候删掉这个推导,类似 https://github.com/Certseeds/CS323_Compilers_2020F/blob/master/project1/src/syntax.y
 * 但是记得加std::move，不然会报错，因为unique_ptr无拷贝构造函数
 * 待实现功能：基本推导，语法错误，词法错误
*/
    ExtDefList {$$=BUILD_AST_NODE(NON_TERMINAL, "Program"); $$->subNodes.push_back(std::move($1));frontage.m_ast.push_back(std::move($$));}
    | error { yyerror("Syntax error", SYNTAX_ERROR); }

ExtDefList: ExtDef ExtDefList {$$=BUILD_AST_NODE(NON_TERMINAL, "ExtDefList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | /* empty */ ;

ExtDef: Specifier ExtDecList SEMI {$$=BUILD_AST_NODE(NON_TERMINAL, "ExtDef"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Specifier ExtDecList error { yyerror ("Syntax error:missing SEMI", SYNTAX_ERROR); }
    | Specifier SEMI {$$=BUILD_AST_NODE(NON_TERMINAL, "ExtDef"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | Specifier FunDec CompSt SEMI error { yyerror ("Syntax error:redundant SEMI", SYNTAX_ERROR); }
    | Specifier error { yyerror ("Syntax error:missing SEMI", SYNTAX_ERROR); }
    | Specifier FunDec CompSt {$$=BUILD_AST_NODE(NON_TERMINAL, "ExtDef"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | error FunDec CompSt { yyerror ("Syntax error:missing Specifier", SYNTAX_ERROR); };

ExtDecList: VarDec {$$=BUILD_AST_NODE(NON_TERMINAL, "ExtDecList"); $$->subNodes.push_back(std::move($1));}
    | VarDec COMMA ExtDecList {$$=BUILD_AST_NODE(NON_TERMINAL, "ExtDecList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));};

Specifier: INT {$$=BUILD_AST_NODE(NON_TERMINAL, "Specifier"); $$->subNodes.push_back(std::move($1));}
    | FLOAT {$$=BUILD_AST_NODE(NON_TERMINAL, "Specifier"); $$->subNodes.push_back(std::move($1));}
    | CHAR {$$=BUILD_AST_NODE(NON_TERMINAL, "Specifier"); $$->subNodes.push_back(std::move($1));}
    | StructSpecifier {$$=BUILD_AST_NODE(NON_TERMINAL, "Specifier"); $$->subNodes.push_back(std::move($1));};

StructSpecifier: STRUCT ID LC DefList RC {
    $$=BUILD_AST_NODE(NON_TERMINAL, "StructSpecifier"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); 
    $$->subNodes.push_back(std::move($4)); $$->subNodes.push_back(std::move($5));}
    | STRUCT ID LC DefList error { yyerror ("Syntax error:missing RC", SYNTAX_ERROR); }
    | STRUCT ID { $$=BUILD_AST_NODE(NON_TERMINAL, "StructSpecifier"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));};


VarDec: ID {$$=BUILD_AST_NODE(NON_TERMINAL, "VarDec"); $$->subNodes.push_back(std::move($1));}
    | VarDec LB INT RB {$$=BUILD_AST_NODE(NON_TERMINAL, "VarDec"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4));}
    | VarDec LB INT error { yyerror ("Syntax error:missing RB", SYNTAX_ERROR); }
    | error { yyerror ("Syntax error:Var can not be Exp", SYNTAX_ERROR); };

FunDec: ID LP VarList RP {$$=BUILD_AST_NODE(NON_TERMINAL, "FunDec"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4));}
    | ID LP RP {$$=BUILD_AST_NODE(NON_TERMINAL, "FunDec"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));};
    | ID LP VarList error { yyerror ("Syntax error:missing RP", SYNTAX_ERROR); }
    | ID LP error { yyerror ("Syntax error:missing RP", SYNTAX_ERROR); };

VarList: ParamDec COMMA VarList {$$=BUILD_AST_NODE(NON_TERMINAL, "VarList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | ParamDec {$$=BUILD_AST_NODE(NON_TERMINAL, "VarList"); $$->subNodes.push_back(std::move($1));};


ParamDec: Specifier VarDec {$$=BUILD_AST_NODE(NON_TERMINAL, "ParamDec"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));};

CompSt: LC DefList StmtList RC {$$=BUILD_AST_NODE(NON_TERMINAL, "CompSt"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4));}
    | LC DecList StmtList error { yyerror ("Syntax error:missing RC", SYNTAX_ERROR); }
    | LC DefList StmtList DefList error { yyerror ("Syntax error:missing specifier", SYNTAX_ERROR); };
    

StmtList: Stmt StmtList {$$=BUILD_AST_NODE(NON_TERMINAL, "StmtList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | /* empty */ ;

Stmt: Exp SEMI {$$=BUILD_AST_NODE(NON_TERMINAL, "Stmt"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | SEMI error { yyerror ("Syntax error:redundant SEMI", SYNTAX_ERROR); }
    | Exp error { yyerror ("Syntax error:missing SEMI", SYNTAX_ERROR); }
    | CompSt {$$=BUILD_AST_NODE(NON_TERMINAL, "Stmt"); $$->subNodes.push_back(std::move($1));}
    | RETURN Exp SEMI {$$=BUILD_AST_NODE(NON_TERMINAL, "Stmt"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | RETURN Exp error { yyerror ("Syntax error:missing SEMI", SYNTAX_ERROR); }
    | IF LP Exp RP Stmt {$$=BUILD_AST_NODE(NON_TERMINAL, "Stmt"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4)); $$->subNodes.push_back(std::move($5));}
    | IF LP Exp RP Stmt ELSE Stmt {$$=BUILD_AST_NODE(NON_TERMINAL, "Stmt"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4)); $$->subNodes.push_back(std::move($5)); $$->subNodes.push_back(std::move($6)); $$->subNodes.push_back(std::move($7));}
    | WHILE LP Exp RP Stmt {$$=BUILD_AST_NODE(NON_TERMINAL, "Stmt"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4)); $$->subNodes.push_back(std::move($5));};

DefList: Def DefList {$$=BUILD_AST_NODE(NON_TERMINAL, "DefList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | /* empty */ ;

Def: Specifier DecList SEMI {$$=BUILD_AST_NODE(NON_TERMINAL, "Def"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Specifier DecList error { yyerror ("Syntax error:missing SEMI", SYNTAX_ERROR); };

DecList: Dec {$$=BUILD_AST_NODE(NON_TERMINAL, "DecList"); $$->subNodes.push_back(std::move($1));}
    | Dec COMMA DecList {$$=BUILD_AST_NODE(NON_TERMINAL, "DecList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));};

Dec: VarDec {$$=BUILD_AST_NODE(NON_TERMINAL, "Dec"); $$->subNodes.push_back(std::move($1));}
    | VarDec ASSIGN Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Dec"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));};

Exp: Exp ASSIGN Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp AND Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp OR Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp LT Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp LE Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp GT Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp GE Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp NE Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp EQ Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp PLUS Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp MINUS Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp MUL Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp DIV Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | LP Exp RP {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | LP Exp error { yyerror ("Syntax error:missing RP", SYNTAX_ERROR); }
    | MINUS Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | NOT Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | ID LP Args RP {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4));}
    | ID LP Args error { yyerror ("Syntax error:missing RP", SYNTAX_ERROR); }
    | ID LP RP {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | ID LP error { yyerror ("Syntax error:missing RP", SYNTAX_ERROR); }
    | Exp LB Exp RB {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4));}
    | Exp LB Exp error { yyerror ("Syntax error:missing RB", SYNTAX_ERROR); }
    | Exp DOT ID {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | ID {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1));}
    | INT {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1));}
    | FLOAT {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1));}
    | CHAR {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1));};

Args: Exp COMMA Args {$$=BUILD_AST_NODE(NON_TERMINAL, "Args"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Args"); $$->subNodes.push_back(std::move($1));}

%%
void spl::Parser::error(const location &loc , const std::string &message) {
        cout << "Error: " << message << "\nError location: " << frontage.location() << '\n';
}

void yyerror(const char* msg, error_type err) {
    switch(err) {
        case LEXICAL_ERROR:
            std::cerr << "Lexical error: " << msg << std::endl;
            break;
        case SYNTAX_ERROR:
            std::cerr << "Syntax error: " << msg << std::endl;
            break;
        case SEMANTIC_ERROR:
            std::cerr << "Semantic error: " << msg << std::endl;
            break;
        case OTHER_ERROR:
            std::cerr << "Other error: " << msg << std::endl;
            break;
    }
}
//TODO: 加入词法，语义错误的处理，显示行号等等信息 @JYF
