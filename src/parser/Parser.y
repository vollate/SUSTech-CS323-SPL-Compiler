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
    #include <cstdint>
    #include "Command.hpp"

    using namespace std;

    namespace spl {
        class Scanner;
        class Frontage;
    }
}

// Bison calls yylex() function that must be provided by us to suck tokens
// from the scanner. This block will be placed at the beginning of IMPLEMENTATION file (cpp).
// We define this function here (function! not method).
// This function is called only inside Bison, so we make it static to limit symbol visibility for the linker
// to avoid potential linking conflicts.
%code top
{
    #include "Scanner.hpp"
    #include "Parser.hpp"
    #include "Frontage.hpp"
    #include "location.hh"
    
    static spl::Parser::symbol_type yylex(spl::Scanner &scanner, spl::Frontage &frontage) {
        return scanner.get_next_token();
    }
    
    // you can accomplish the same thing by inlining the code using preprocessor
    // x and y are same as in above static function
    // #define yylex(x, y) scanner.get_next_token()
    
    using namespace spl;
}

%lex-param { spl::Scanner &scanner }
%lex-param { spl::Frontage &frontage }
%parse-param { spl::Scanner &scanner }
%parse-param { spl::Frontage &frontage }
%locations
%define parse.trace
%define parse.error verbose

%define api.token.prefix {TOKEN_}

%token <int32_t> INT "int";
%token <float> FLOAT "float";
%token <char> CHAR "char";
%token <std::string> ID;
%token <std::string> TYPE "type";
%token <std::string> STRUCT "struct";
%token <std::string> IF "if";
%token <std::string> ELSE "else";
%token <std::string> WHILE "while";
%token <std::string> RETURN "return";
%token <std::string> DOT ".";
%token <std::string> SEMI ";";
%token <std::string> COMMA ",";
%token <std::string> ASSIGN "=";
%token <std::string> LT "<";
%token <std::string> LE "<=";
%token <std::string> GT ">";
%token <std::string> GE ">=";
%token <std::string> NE "!=";
%token <std::string> EQ "==";
%token <std::string> PLUS "+";
%token <std::string> MINUS "-";
%token <std::string> MUL "*";
%token <std::string> DIV "/";
%token <std::string> AND "&&";
%token <std::string> OR "||";
%token <std::string> NOT "!";
%token <std::string> LP "(";
%token <std::string> RP ")"
%token <std::string> LB "[";
%token <std::string> RB "]";
%token <std::string> LC "{";
%token <std::string> RC "}";
%token TERMINATE;


//%type< spl::Command > command;
//%type< std::vector<uint64_t> > arguments;


%%

Program: ExtDefList {
    frontage.addCommand("Program", $1);
}
| error {printf("Error 0 waited to define\n");};


ExtDefList: ExtDef ExtDefList {
}
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

// Bison expects us to provide implementation - otherwise linker complains
void spl::Parser::error(const location &loc , const std::string &message) {
        
        // Location should be initialized inside scanner action, but is not in this example.
        // Let's grab location directly from frontage class.
	// cout << "Error: " << message << endl << "Location: " << loc << endl;
	
        cout << "Error: " << message << endl << "Error location: " << frontage.location() << endl;
}
