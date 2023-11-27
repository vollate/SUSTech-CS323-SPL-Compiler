%language "c++"
%require "3.8"
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
    #include <variant>
    #include <memory>
    #include <sstream>
    #include <type_traits>
    #include <cstdint>
    #include <list>
    #include "location.hh"

    using namespace std;

    enum class ERROR_TYPE {
        LEXICAL_ERROR,
        SYNTAX_ERROR,
        OTHER_ERROR
    };


    namespace spl {
        class Scanner;
        class Frontage;
        class Parser;
        struct ParseNode {
            using variant_type = std::variant<int32_t, float, std::string>;

            int32_t type;
            variant_type value;
            location loc;
            std::list<std::unique_ptr<ParseNode>> subNodes;

            ParseNode()=default;
            ParseNode(const ParseNode&rhs)=delete;
            ParseNode& operator=(const ParseNode&rhs)=delete;
            ParseNode(int32_t type,location& loc, variant_type &&value) : type(type),loc(std::forward<spl::location>(loc)), value(std::forward<variant_type>(value)) {}
            ParseNode(int32_t type,location&& loc, variant_type &&value) : type(type),loc(std::forward<spl::location>(loc)), value(std::forward<variant_type>(value)) {}
            void addSubNodes(std::vector<std::unique_ptr<ParseNode>>&& valList){
                for(auto&& node: valList){
                    subNodes.push_back(std::move(node));
                }
            }
        };
    }
    using NodeType = std::unique_ptr<spl::ParseNode>;
}

%code top
{
    #include "Scanner.hpp"
    #include "Parser.hpp"
    #include "Frontage.hpp"

    static spl::Parser::symbol_type yylex(spl::Scanner &scanner, spl::Frontage &frontage) {
        return scanner.next_token();
    }

    using namespace spl;
    using token_type = Parser::token_type;
    #define BUILD_AST_NODE(__type,__line,__value)\
    	std::make_unique<ParseNode>(token_type::__type,__line, __value)

    void yyerror(const std::string& msg,spl::location&loc, ERROR_TYPE err,spl::Frontage&frontage);
    
    template<typename T>
    static void move_one(T& vec) {}

    template<typename T, typename First, typename... Args>
    static void move_one(T& vec, First&& val, Args&&... args) {
        vec.push_back(std::move(val));
        move_one(vec, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static auto move_all(Args&&... args) {
        static_assert(sizeof...(Args)>0,"Argument length can't be zero");
        using T = typename std::remove_reference<typename std::tuple_element<0, std::tuple<Args...>>::type>::type;
        std::vector<T> res;
        move_one(res, std::forward<Args>(args)...);
        return res;
    }
}

%lex-param { spl::Scanner &scanner }
%lex-param { spl::Frontage &frontage }
%parse-param { spl::Scanner &scanner }
%parse-param { spl::Frontage &frontage }
%locations
//%define parse.trace
%define parse.error verbose

%token END 0 "end of file"
%nonassoc <NodeType> ILLEGAL_TOKEN
%nonassoc <NodeType> LOW_THAN_ELSE
%nonassoc <NodeType> ELSE
%token <NodeType> INCLUDE
%token <NodeType> TYPE STRUCT
%token <NodeType> IF WHILE RETURN
%token <NodeType> INT
%token <NodeType> FLOAT
%token <NodeType> CHAR
%token <NodeType> ID
%right <NodeType> ASSIGN
%left <NodeType> OR
%left <NodeType> AND
%left <NodeType> LT LE GT GE NE EQ
%nonassoc LOWER_PLUS LOWER_MINUS
%left <NodeType> PLUS MINUS
%left <NodeType> PLUS_EQUAL MINUS_EQUAL
%left <NodeType> MUL DIV
%left <NodeType> MUL_EQUAL DIV_EQUAL
%right <NodeType> NOT
%left <NodeType> LP RP LB RB DOT
%token <NodeType> SEMI COMMA
%token <NodeType> LC RC

%type <NodeType> Program ExtDefList
%type <NodeType> ExtDef ExtDecList Specifier StructSpecifier VarDec
%type <NodeType> FunDec VarList ParamDec CompSt StmtList Stmt DefList
%type <NodeType> Def DecList Dec Args Exp

%nonassoc <NodeType> LEXICAL_ERROR OTHER_ERROR

//%nonassoc <NodeType> ERROR_TYPE::SYNTAX_ERROR;

//!!! just used to pass value, do not use this in deduction
%nonassoc <std::string> NON_TERMINAL
%nonassoc <int32_t> NOTHING

%%

Program: ExtDefList {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Program");$$->addSubNodes(move_all($1));frontage.m_ast.push_back(std::move($$)); }
    | INCLUDE {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$,"Program");$$->addSubNodes(move_all($1));frontage.m_ast.push_back(std::move($$));}
    | OTHER_ERROR {@$=@1; yyerror(std::get<std::string>($1->value),@$,ERROR_TYPE::OTHER_ERROR,frontage);}
    | {$$=BUILD_AST_NODE(NOTHING,@$, 0);};

ExtDefList: ExtDef ExtDefList {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "ExtDefList"); $$->addSubNodes(move_all($1,$2));}
    | {$$=BUILD_AST_NODE(NOTHING,@$,0);};

ExtDef: Specifier ExtDecList SEMI {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "ExtDef"); $$->addSubNodes(move_all($1,$2,$3));}
    | Specifier SEMI {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "ExtDef"); $$->addSubNodes(move_all($1,$2));}
    | Specifier FunDec CompSt {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "ExtDef"); $$->addSubNodes(move_all($1,$2,$3));}
    | Specifier ExtDecList error {@$=@1; yyerror ("missing ';'", @$,ERROR_TYPE::SYNTAX_ERROR,frontage);}
    | Specifier error {@$=@1; yyerror ("missing ';'",@$, ERROR_TYPE::SYNTAX_ERROR,frontage); }
    | error SEMI {@$=@1; yyerror ("missing specifier",@$, ERROR_TYPE::SYNTAX_ERROR,frontage); }

ExtDecList: VarDec {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL, @$,"ExtDecList"); $$->addSubNodes(move_all($1));}
    | VarDec COMMA ExtDecList {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "ExtDecList"); $$->addSubNodes(move_all($1,$2,$3));}
    | VarDec COMMA error {@$=@1; yyerror ("missing ExtDecList",@$, ERROR_TYPE::SYNTAX_ERROR,frontage); }
   // | VarDec error ExtDecList  {@$=@1;yyerror("missing ','",@$, ERROR_TYPE::SYNTAX_ERROR,frontage);}

Specifier: TYPE {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Specifier"); $$->addSubNodes(move_all($1));}
    | StructSpecifier {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL, @$,"Specifier"); $$->addSubNodes(move_all($1));};

StructSpecifier: STRUCT ID LC DefList RC {@$=@1; $$=BUILD_AST_NODE(NON_TERMINAL,@$, "StructSpecifier"); $$->addSubNodes(move_all($1,$2,$3,$4,$5));}
    | STRUCT ID {@$=@1; $$=BUILD_AST_NODE(NON_TERMINAL,@$, "StructSpecifier"); $$->addSubNodes(move_all($1,$2));}
    | STRUCT ID LC DefList error {@$=@1; yyerror ("missing '}'",@$, ERROR_TYPE::SYNTAX_ERROR,frontage); }

VarDec: ID {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "VarDec"); $$->addSubNodes(move_all($1));}
    | VarDec LB INT RB {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "VarDec"); $$->addSubNodes(move_all($1,$2,$3,$4));}
    | VarDec LB INT error {@$=@1; yyerror ("missing ']'",@$, ERROR_TYPE::SYNTAX_ERROR,frontage);}
    | LEXICAL_ERROR {@$=@1;yyerror(std::get<std::string>($1->value),@$,ERROR_TYPE::LEXICAL_ERROR,frontage);}

FunDec: ID LP VarList RP {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "FunDec"); $$->addSubNodes(move_all($1,$2,$3,$4));}
    | ID LP RP {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "FunDec"); $$->addSubNodes(move_all($1,$2,$3));};
    | ID LP VarList error {@$=@1; yyerror ("missing ')'",@$, ERROR_TYPE::SYNTAX_ERROR,frontage); }
    | ID LP error {@$=@1; yyerror ("missing ')'",@$, ERROR_TYPE::SYNTAX_ERROR,frontage); }

VarList: ParamDec COMMA VarList {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "VarList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | ParamDec {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "VarList"); $$->addSubNodes(move_all($1));}
    | ParamDec COMMA error {@$=@1;yyerror("missing arguments",@$,ERROR_TYPE::SYNTAX_ERROR,frontage);}


ParamDec: Specifier VarDec {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "ParamDec"); $$->addSubNodes(move_all($1,$2));};
    | error VarDec {@$=@1;yyerror("missing specifier",@$,ERROR_TYPE::SYNTAX_ERROR,frontage);}
    | Specifier error {@$=@1;yyerror("missing variable",@$,ERROR_TYPE::SYNTAX_ERROR,frontage);}

CompSt: LC DefList StmtList RC {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "CompSt"); $$->addSubNodes(move_all($1,$2,$3,$4));}
    //| LC DefList StmtList error {@$=@1; yyerror ("missing '}'",@$, ERROR_TYPE::SYNTAX_ERROR,frontage); }


StmtList: Stmt StmtList {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "StmtList"); $$->addSubNodes(move_all($1,$2));}
    | {$$=BUILD_AST_NODE(NOTHING,@$, 0);}

Stmt: Exp SEMI {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Stmt"); $$->addSubNodes(move_all($1,$2));}
    | CompSt {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Stmt"); $$->subNodes.push_back(std::move($1));}
    | RETURN Exp SEMI {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Stmt");$$->addSubNodes(move_all($1,$2,$3));}
    | IF LP Exp RP Stmt ELSE Stmt {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Stmt"); $$->addSubNodes(move_all($1,$2,$3,$4,$5,$6,$7));}
    | IF LP Exp RP Stmt %prec LOW_THAN_ELSE {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Stmt"); $$->addSubNodes(move_all($1,$2,$3,$4,$5));}
    | WHILE LP Exp RP Stmt {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Stmt"); $$->addSubNodes(move_all($1,$2,$3,$4,$5));}
    | WHILE LP Exp error Stmt {@$=@1;yyerror("missing ')'",@$,ERROR_TYPE::SYNTAX_ERROR,frontage);}
    //| Exp error {@$=@1;yyerror("missing ';'",@$,ERROR_TYPE::SYNTAX_ERROR,frontage);}
    | RETURN Exp error {@$=@1;yyerror("missing ';'",@$,ERROR_TYPE::SYNTAX_ERROR,frontage);}
    | IF LP Exp error Stmt  {@$=@1;yyerror("missing ')'",@$,ERROR_TYPE::SYNTAX_ERROR,frontage);}
    | IF error Exp RP Stmt {@$=@1;yyerror("missing '('",@$,ERROR_TYPE::SYNTAX_ERROR,frontage); }

DefList: Def DefList {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "DefList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | {$$=BUILD_AST_NODE(NOTHING,@$, 0);}

Def: Specifier DecList SEMI {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Def"); $$->addSubNodes(move_all($1,$2,$3));}
    | Specifier DecList error {@$=@1; yyerror ("missing ';'",@$, ERROR_TYPE::SYNTAX_ERROR,frontage); }
    |error DecList SEMI {@$=@1; yyerror ("missing specifier",@$, ERROR_TYPE::SYNTAX_ERROR,frontage); }


DecList: Dec {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "DecList"); $$->addSubNodes(move_all($1));}
    | Dec COMMA DecList {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "DecList"); $$->addSubNodes(move_all($1,$2,$3));}
    | Dec COMMA error {@$=@1; yyerror ("missing DecList",@$, ERROR_TYPE::SYNTAX_ERROR,frontage); }
    //| Dec error DecList {@$=@1;yyerror("missing ','",@$,ERROR_TYPE::SYNTAX_ERROR,frontage);}

Dec: VarDec {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Dec"); $$->addSubNodes(move_all($1));}
    | VarDec ASSIGN Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Dec"); $$->addSubNodes(move_all($1,$2,$3));};

Exp: Exp ASSIGN Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp");$$->addSubNodes(move_all($1,$2,$3));}
    | Exp AND Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3));}
    | Exp OR Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3));}
    | Exp LT Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3));}
    | Exp LE Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3));}
    | Exp GT Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3));}
    | Exp GE Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3));}
    | Exp NE Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3));}
    | Exp EQ Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3));}
    | Exp PLUS Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3));}
    | Exp MINUS Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3));}
    | Exp PLUS_EQUAL Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3));}
    | Exp MINUS_EQUAL Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3));}
    | Exp MUL Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3));}
    | Exp DIV Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3));}
    | Exp MUL_EQUAL Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3));}
    | Exp DIV_EQUAL Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3));}
    | LP Exp RP {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL, @$,"Exp"); $$->addSubNodes(move_all($1,$2,$3));}
    | LP Exp error {@$=@1; yyerror ("missing ')'",@$, ERROR_TYPE::SYNTAX_ERROR,frontage); }
    | MINUS Exp %prec LOWER_MINUS {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2));}
    | PLUS Exp %prec LOWER_PLUS {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2));}
    | NOT Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2));}
    | ID LP Args RP {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3,$4));}
    | ID LP Args error {@$=@1; yyerror ("missing ')'",@$, ERROR_TYPE::SYNTAX_ERROR,frontage); }
    | ID LP RP {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3));}
    //| ID LP error {@$=@1; yyerror ("missing ')'",@$, ERROR_TYPE::SYNTAX_ERROR,frontage); }
    | Exp LB Exp RB {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3,$4));}
    | Exp LB Exp error {@$=@1; yyerror ("missing ']'",@$, ERROR_TYPE::SYNTAX_ERROR,frontage); }
    | Exp DOT ID {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1,$2,$3));}
    | Exp DOT error {@$=@1; yyerror ("missing ID",@$, ERROR_TYPE::SYNTAX_ERROR,frontage); }
    | ID {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1));}
    | INT {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp");$$->addSubNodes(move_all($1));}
    | FLOAT {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1));}
    | CHAR {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->addSubNodes(move_all($1));}
    | Exp LEXICAL_ERROR Exp {@$=@1;yyerror(std::get<std::string>($2->value),@2,ERROR_TYPE::LEXICAL_ERROR,frontage);}
    | LEXICAL_ERROR {@$=@1;yyerror(std::get<std::string>($1->value),@$,ERROR_TYPE::LEXICAL_ERROR,frontage);}

Args: Exp COMMA Args {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Args"); $$->addSubNodes(move_all($1,$2,$3));}
    | Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Args"); $$->addSubNodes(move_all($1));}
    //| Exp error Args {@$=@1;yyerror("missing ','",@$,ERROR_TYPE::SYNTAX_ERROR,frontage);}

%%
void spl::Parser::error(const location &loc , const std::string &message) {
       // cout << "Error: " << message << "\nError location: " << frontage.location() << '\n';
}

void yyerror(const std::string& msg,spl::location&loc, ERROR_TYPE err, spl::Frontage &frontage) {
    std::stringstream ss;
    switch(err) {
        case ERROR_TYPE::LEXICAL_ERROR:
            ss << "Error type A at Line " <<loc.end.line<<":"<< msg ;
            break;
        case ERROR_TYPE::SYNTAX_ERROR:
            ss << "Error type B at Line " << loc.end.line<<":"<<msg ;
            break;
        case ERROR_TYPE::OTHER_ERROR:
            ss << "Other error: " << msg ;
            break;
    }
    #ifdef SPL_DEBUG
    std::cout << ss.str() << '\n';
    #endif
    frontage.appendError(ss.str());
}
