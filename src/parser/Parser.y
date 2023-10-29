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
        struct ASTNode {
            using variant_type = std::variant<int32_t, float, std::string>;

            int32_t type;
            variant_type value;
            location loc;
            std::list<std::unique_ptr<ASTNode>> subNodes;
            ASTNode(int32_t type,location& loc, variant_type &&value) : type(type),loc(std::forward<spl::location>(loc)), value(std::forward<variant_type>(value)) {}
            ASTNode(int32_t type,location&& loc, variant_type &&value) : type(type),loc(std::forward<spl::location>(loc)), value(std::forward<variant_type>(value)) {
            #ifdef SPL_DEBUG
            cout<<"new node location: "<<loc<<'\n';
            #endif
            }
        };
    }
    using NodeType = std::unique_ptr<spl::ASTNode>;
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
    	std::make_unique<ASTNode>(token_type::__type,__line, __value)
    void yyerror(const std::string& msg, ERROR_TYPE err,spl::Frontage&frontage);
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
%nonassoc LOWER_MINUS
%left <NodeType> PLUS MINUS
%left <NodeType> MUL DIV
%right <NodeType> NOT
%left <NodeType> LP RP LB RB DOT
%token <NodeType> SEMI COMMA
%token <NodeType> LC RC

%type <NodeType> Program ExtDefList
%type <NodeType> ExtDef ExtDecList Specifier StructSpecifier VarDec
%type <NodeType> FunDec VarList ParamDec CompSt StmtList Stmt DefList
%type <NodeType> Def DecList Dec Args Exp
%nonassoc <NodeType> LEXICAL_ERROR;

//%nonassoc <NodeType> ERROR_TYPE::SYNTAX_ERROR;

//!!! just used to pass value, do not use this in deduction
%nonassoc <std::string> NON_TERMINAL
%nonassoc <int32_t> NOTHING

%%

Program: ExtDefList {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Program"); $$->subNodes.push_back(std::move($1));frontage.m_ast.push_back(std::move($$)); }
    | {$$=BUILD_AST_NODE(NOTHING,@$, 0);};

ExtDefList: ExtDef ExtDefList {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "ExtDefList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | {$$=BUILD_AST_NODE(NOTHING,@$,0);};

ExtDef: Specifier ExtDecList SEMI {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "ExtDef"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Specifier SEMI {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "ExtDef"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | Specifier FunDec CompSt {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "ExtDef"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Specifier ExtDecList error {@$=@1; yyerror ("redundant SEMI", ERROR_TYPE::SYNTAX_ERROR,frontage);}
    | Specifier error {@$=@1; yyerror ("missing SEMI", ERROR_TYPE::SYNTAX_ERROR,frontage); }
    ;

ExtDecList: VarDec {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL, @$,"ExtDecList"); $$->subNodes.push_back(std::move($1));}
    | VarDec COMMA ExtDecList {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "ExtDecList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | VarDec error ExtDecList  {@$=@1;yyerror("missing COMMA", ERROR_TYPE::SYNTAX_ERROR,frontage);}
    ;

Specifier: TYPE {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Specifier"); $$->subNodes.push_back(std::move($1));}
    | StructSpecifier {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL, @$,"Specifier"); $$->subNodes.push_back(std::move($1));};

StructSpecifier: STRUCT ID LC DefList RC {@$=@1; $$=BUILD_AST_NODE(NON_TERMINAL,@$, "StructSpecifier"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4)); $$->subNodes.push_back(std::move($5));}
    | STRUCT ID {@$=@1; $$=BUILD_AST_NODE(NON_TERMINAL,@$, "StructSpecifier"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | STRUCT ID LC DefList error {@$=@1; yyerror ("missing RC", ERROR_TYPE::SYNTAX_ERROR,frontage); }
    ;

VarDec: ID {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "VarDec"); $$->subNodes.push_back(std::move($1));}
    | VarDec LB INT RB {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "VarDec"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4));}
    | VarDec LB INT error {@$=@1; yyerror ("missing RB", ERROR_TYPE::SYNTAX_ERROR,frontage);/*TODO*/}
    | error {@$=@1; yyerror ("Var can not be Exp", ERROR_TYPE::SYNTAX_ERROR,frontage);/*useless?*/};

FunDec: ID LP VarList RP {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "FunDec"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4));}
    | ID LP RP {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "FunDec"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));};
    | ID LP VarList error {@$=@1; yyerror ("missing RP", ERROR_TYPE::SYNTAX_ERROR,frontage); }
    | ID LP error {@$=@1; yyerror ("missing RP", ERROR_TYPE::SYNTAX_ERROR,frontage); };

VarList: ParamDec COMMA VarList {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "VarList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | ParamDec {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "VarList"); $$->subNodes.push_back(std::move($1));}
    | ParamDec VarList error {@$=@1;yyerror("missing COMMA",ERROR_TYPE::SYNTAX_ERROR,frontage);}
    ;


ParamDec: Specifier VarDec {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "ParamDec"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));};

CompSt: LC DefList StmtList RC {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "CompSt"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4));}
    ;


StmtList: Stmt StmtList {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "StmtList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | {$$=BUILD_AST_NODE(NOTHING,@$, 0);};

Stmt: Exp SEMI {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Stmt"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | CompSt {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Stmt"); $$->subNodes.push_back(std::move($1));}
    | RETURN Exp SEMI {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Stmt"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | IF LP Exp RP Stmt ELSE Stmt {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Stmt"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4)); $$->subNodes.push_back(std::move($5)); $$->subNodes.push_back(std::move($6)); $$->subNodes.push_back(std::move($7));}
    | IF LP Exp RP Stmt %prec LOW_THAN_ELSE {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Stmt"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4)); $$->subNodes.push_back(std::move($5));}
    | WHILE LP Exp RP Stmt {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Stmt"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4)); $$->subNodes.push_back(std::move($5));}
    | WHILE LP Exp error Stmt {@$=@1;yyerror("lack of RP",ERROR_TYPE::SYNTAX_ERROR,frontage);}
    | Exp error {@$=@1;yyerror("missing SEMI",ERROR_TYPE::SYNTAX_ERROR,frontage);}
    | RETURN Exp error {@$=@1;yyerror("missing SEMI",ERROR_TYPE::SYNTAX_ERROR,frontage);}
    | IF LP Exp error Stmt  {@$=@1;yyerror("missing RP",ERROR_TYPE::SYNTAX_ERROR,frontage);}
    | IF error Exp RP Stmt {@$=@1;yyerror("missing LP",ERROR_TYPE::SYNTAX_ERROR,frontage); }
    ;

DefList: Def DefList {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "DefList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | {$$=BUILD_AST_NODE(NOTHING,@$, 0);};

Def: Specifier DecList SEMI {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Def"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Specifier DecList error {@$=@1; yyerror ("missing SEMI", ERROR_TYPE::SYNTAX_ERROR,frontage); }
    | error DecList SEMI {@$=@1;yyerror("missing SPEC",ERROR_TYPE::SYNTAX_ERROR,frontage);}
    ;

DecList: Dec {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "DecList"); $$->subNodes.push_back(std::move($1));}
    | Dec COMMA DecList {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "DecList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Dec DecList error {@$=@1;yyerror("missing COMMA",ERROR_TYPE::SYNTAX_ERROR,frontage);}
    ;

Dec: VarDec {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Dec"); $$->subNodes.push_back(std::move($1));}
    | VarDec ASSIGN Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Dec"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));};

Exp: Exp ASSIGN Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp AND Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp OR Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp LT Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp LE Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp GT Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp GE Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp NE Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp EQ Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp PLUS Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp MINUS Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp MUL Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp DIV Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | LP Exp RP {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL, @$,"Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | LP Exp error {@$=@1; yyerror ("missing RP", ERROR_TYPE::SYNTAX_ERROR,frontage); }
    | MINUS Exp %prec LOWER_MINUS {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | NOT Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | ID LP Args RP {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4));}
    | ID LP Args error {@$=@1; yyerror ("missing RP", ERROR_TYPE::SYNTAX_ERROR,frontage); }
    | ID LP RP {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | ID LP error {@$=@1; yyerror ("missing RP", ERROR_TYPE::SYNTAX_ERROR,frontage); }
    | Exp LB Exp RB {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4));}
    | Exp LB Exp error {@$=@1; yyerror ("missing RB", ERROR_TYPE::SYNTAX_ERROR,frontage); }
    | Exp DOT ID {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | ID {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1));}
    | INT {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1));}
    | FLOAT {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1));}
    | CHAR {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Exp"); $$->subNodes.push_back(std::move($1));}
    | Exp LEXICAL_ERROR Exp {@$=@1;yyerror(std::get<std::string>($2->value),ERROR_TYPE::LEXICAL_ERROR,frontage);}
    | LEXICAL_ERROR {yyerror(std::get<std::string>($1->value),ERROR_TYPE::LEXICAL_ERROR,frontage);}
    ;

Args: Exp COMMA Args {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Args"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp {@$=@1;$$=BUILD_AST_NODE(NON_TERMINAL,@$, "Args"); $$->subNodes.push_back(std::move($1));}
%%
void spl::Parser::error(const location &loc , const std::string &message) {
        cout << "Error: " << message << "\nError location: " << frontage.location() << '\n';
}

void yyerror(const std::string& msg, ERROR_TYPE err, spl::Frontage &frontage) {
    std::stringstream ss;
    switch(err) {
        case ERROR_TYPE::LEXICAL_ERROR:
            ss << "Error type A at Line " <<frontage.location().end.line<<" :"<< msg ;
            break;
        case ERROR_TYPE::SYNTAX_ERROR:
            ss << "Error type B at Line " << frontage.location().end.line<<" :"<<msg ;
            break;
        case ERROR_TYPE::OTHER_ERROR:
            ss << "Other error: " << msg ;
            break;
    }
    frontage.appendError(ss.str());
}
