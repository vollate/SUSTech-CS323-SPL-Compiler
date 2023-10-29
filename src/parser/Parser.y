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
    #include "location.hh"

    using namespace std;

    enum class ERROR_TYPE {
        LEXICAL_ERROR,
        SYNTAX_ERROR,
        SEMANTIC_ERROR,
        OTHER_ERROR
    };

    void yyerror(const char* msg, ERROR_TYPE err);

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
}

%code top
{
    #include "Scanner.hpp"
    #include "Parser.hpp"
    #include "Frontage.hpp"

    static spl::Parser::symbol_type yylex(spl::Scanner &scanner, spl::Frontage &frontage) {
        return scanner.next_token();
    }
    // you can accomplish the same thing by inlining the code using preprocessor
    // x and y are same as in above static function
    // #define yylex(x, y) scanner.get_next_token()
    
    using namespace spl;
    using token_type = Parser::token_type;
    #define BUILD_AST_NODE(__type,__value)\
    	std::make_unique<ASTNode>(token_type::__type,frontage.location(), __value)
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

%%

Program: ExtDefList {$$=BUILD_AST_NODE(NON_TERMINAL, "Program"); $$->subNodes.push_back(std::move($1));frontage.m_ast.push_back(std::move($$));}
    |  { $$=BUILD_AST_NODE(NON_TERMINAL, "Program");};

ExtDefList: ExtDef ExtDefList {$$=BUILD_AST_NODE(NON_TERMINAL, "ExtDefList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | {$$=BUILD_AST_NODE(NON_TERMINAL, "ExtDefList");};

ExtDef: Specifier ExtDecList SEMI {$$=BUILD_AST_NODE(NON_TERMINAL, "ExtDef"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Specifier SEMI {$$=BUILD_AST_NODE(NON_TERMINAL, "ExtDef"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | Specifier FunDec CompSt {$$=BUILD_AST_NODE(NON_TERMINAL, "ExtDef"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Specifier FunDec CompSt SEMI error { yyerror ("Syntax error:redundant SEMI", ERROR_TYPE::SYNTAX_ERROR); }
    ;
ExtDecList: VarDec {$$=BUILD_AST_NODE(NON_TERMINAL, "ExtDecList"); $$->subNodes.push_back(std::move($1));}
    | VarDec COMMA ExtDecList {$$=BUILD_AST_NODE(NON_TERMINAL, "ExtDecList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | VarDec ExtDecList error {yyerror("missing comma", ERROR_TYPE::SYNTAX_ERROR);}
    ;
Specifier: TYPE {$$=BUILD_AST_NODE(NON_TERMINAL, "Specifier"); $$->subNodes.push_back(std::move($1));}
    | StructSpecifier {$$=BUILD_AST_NODE(NON_TERMINAL, "Specifier"); $$->subNodes.push_back(std::move($1));};

StructSpecifier: STRUCT ID LC DefList RC { $$=BUILD_AST_NODE(NON_TERMINAL, "StructSpecifier"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4)); $$->subNodes.push_back(std::move($5));}
    | STRUCT ID { $$=BUILD_AST_NODE(NON_TERMINAL, "StructSpecifier"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | STRUCT ID LC DefList error { yyerror ("Syntax error:missing RC", ERROR_TYPE::SYNTAX_ERROR); }
    ;

VarDec: ID {$$=BUILD_AST_NODE(NON_TERMINAL, "VarDec"); $$->subNodes.push_back(std::move($1));}
    | VarDec LB INT RB {$$=BUILD_AST_NODE(NON_TERMINAL, "VarDec"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4));}
    | VarDec LB INT error { yyerror ("Syntax error:missing RB", ERROR_TYPE::SYNTAX_ERROR); /*TODO*/}
    | error { yyerror ("Syntax error:Var can not be Exp", ERROR_TYPE::SYNTAX_ERROR);/*useless?*/ };

FunDec: ID LP VarList RP {$$=BUILD_AST_NODE(NON_TERMINAL, "FunDec"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4));}
    | ID LP RP {$$=BUILD_AST_NODE(NON_TERMINAL, "FunDec"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));};
    | ID LP VarList error { yyerror ("Syntax error:missing RP", ERROR_TYPE::SYNTAX_ERROR); }
    | ID LP error { yyerror ("Syntax error:missing RP", ERROR_TYPE::SYNTAX_ERROR); };

VarList: ParamDec COMMA VarList {$$=BUILD_AST_NODE(NON_TERMINAL, "VarList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | ParamDec {$$=BUILD_AST_NODE(NON_TERMINAL, "VarList"); $$->subNodes.push_back(std::move($1));}
    | ParamDec VarList error {yyerror("missing comma",ERROR_TYPE::SYNTAX_ERROR);}
    ;


ParamDec: Specifier VarDec {$$=BUILD_AST_NODE(NON_TERMINAL, "ParamDec"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));};

CompSt: LC DefList StmtList RC {$$=BUILD_AST_NODE(NON_TERMINAL, "CompSt"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4));}
    ;//| LC DecList StmtList error { yyerror ("Syntax error:missing RC", ERROR_TYPE::SYNTAX_ERROR); }
    //| LC DefList StmtList DefList error { yyerror ("Syntax error:missing specifier", ERROR_TYPE::SYNTAX_ERROR); };
    

StmtList: Stmt StmtList {$$=BUILD_AST_NODE(NON_TERMINAL, "StmtList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | {$$=BUILD_AST_NODE(NON_TERMINAL, "StmtList");};

Stmt: Exp SEMI {$$=BUILD_AST_NODE(NON_TERMINAL, "Stmt"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | CompSt {$$=BUILD_AST_NODE(NON_TERMINAL, "Stmt"); $$->subNodes.push_back(std::move($1));}
    | RETURN Exp SEMI {$$=BUILD_AST_NODE(NON_TERMINAL, "Stmt"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | IF LP Exp RP Stmt ELSE Stmt {$$=BUILD_AST_NODE(NON_TERMINAL, "Stmt"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4)); $$->subNodes.push_back(std::move($5)); $$->subNodes.push_back(std::move($6)); $$->subNodes.push_back(std::move($7));}
    | IF LP Exp RP Stmt %prec LOW_THAN_ELSE {$$=BUILD_AST_NODE(NON_TERMINAL, "Stmt"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4)); $$->subNodes.push_back(std::move($5));}
    | WHILE LP Exp RP Stmt {$$=BUILD_AST_NODE(NON_TERMINAL, "Stmt"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4)); $$->subNodes.push_back(std::move($5));}
    | WHILE LP Exp error Stmt {yyerror("lack of RP",ERROR_TYPE::SYNTAX_ERROR);}
    | Exp error {yyerror("missing semi",ERROR_TYPE::SYNTAX_ERROR);}
    | RETURN Exp error {yyerror("missing semi",ERROR_TYPE::SYNTAX_ERROR);}
    | IF LP Exp error Stmt  {yyerror("missing RP",ERROR_TYPE::SYNTAX_ERROR);}
    | IF error Exp RP Stmt {yyerror("missing LP",ERROR_TYPE::SYNTAX_ERROR); }
    ;

DefList: Def DefList {$$=BUILD_AST_NODE(NON_TERMINAL, "DefList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | {$$=BUILD_AST_NODE(NON_TERMINAL, "DefList");};

Def: Specifier DecList SEMI {$$=BUILD_AST_NODE(NON_TERMINAL, "Def"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Specifier DecList error { yyerror ("Syntax error:missing SEMI5", ERROR_TYPE::SYNTAX_ERROR); }
    | error DecList SEMI {yyerror("missing SPEC",ERROR_TYPE::SYNTAX_ERROR);}
    ;

DecList: Dec {$$=BUILD_AST_NODE(NON_TERMINAL, "DecList"); $$->subNodes.push_back(std::move($1));}
    | Dec COMMA DecList {$$=BUILD_AST_NODE(NON_TERMINAL, "DecList"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Dec DecList error {yyerror("missing comma",ERROR_TYPE::SYNTAX_ERROR);}
    ;

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
    | LP Exp error { yyerror ("Syntax error:missing RP", ERROR_TYPE::SYNTAX_ERROR); }
    | MINUS Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | NOT Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2));}
    | ID LP Args RP {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4));}
    | ID LP Args error { yyerror ("Syntax error:missing RP", ERROR_TYPE::SYNTAX_ERROR); }
    | ID LP RP {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | ID LP error { yyerror ("Syntax error:missing RP", ERROR_TYPE::SYNTAX_ERROR); }
    | Exp LB Exp RB {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3)); $$->subNodes.push_back(std::move($4));}
    | Exp LB Exp error { yyerror ("Syntax error:missing RB", ERROR_TYPE::SYNTAX_ERROR); }
    | Exp DOT ID {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | ID {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1));}
    | INT {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1));}
    | FLOAT {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1));}
    | CHAR {$$=BUILD_AST_NODE(NON_TERMINAL, "Exp"); $$->subNodes.push_back(std::move($1));}
    ;

Args: Exp COMMA Args {$$=BUILD_AST_NODE(NON_TERMINAL, "Args"); $$->subNodes.push_back(std::move($1)); $$->subNodes.push_back(std::move($2)); $$->subNodes.push_back(std::move($3));}
    | Exp {$$=BUILD_AST_NODE(NON_TERMINAL, "Args"); $$->subNodes.push_back(std::move($1));}

%%
void spl::Parser::error(const location &loc , const std::string &message) {
        cout << "Error: " << message << "\nError location: " << frontage.location() << '\n';
}

void yyerror(const char* msg, ERROR_TYPE err) {
    cerr<<"err: \t"<<msg<<'\n';
    return;
    switch(err) {
        case ERROR_TYPE::LEXICAL_ERROR:
            std::cerr << "Lexical error: " << msg << std::endl;
            break;
        case ERROR_TYPE::SYNTAX_ERROR:
            std::cerr << "Syntax error: " << msg << std::endl;
            break;
        case ERROR_TYPE::SEMANTIC_ERROR:
            std::cerr << "Semantic error: " << msg << std::endl;
            break;
        case ERROR_TYPE::OTHER_ERROR:
            std::cerr << "Other error: " << msg << std::endl;
            break;
    }
}
