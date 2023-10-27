%{
	#include <iostream>
	#include <cstdlib>
	#include "Scanner.hpp"
	#include "Frontage.hpp"
	#include "Parser.hpp"
	#include "location.hh"

	using namespace std;
    using namespace spl;
	#define yyterminate() spl::Parser::make_TERMINATE(spl::location());

	// This will track current scanner location.
	// Action is called when length of the token is known.
	#define YY_USER_ACTION m_frontage.increaseLocation(yyleng);

	// !!!WARNING!!!
	// Location API is used, but the location is not initialized, 'cause I'm lazy. When making
	// a token with make_{something} method you can pass detailed token location. Current location
	// is accessible with m_frontage.location() method. All puzzle elements are there - just
	// pass location value in every action code block below. I'm going to waste more time writing
	// this excuse than putting this boilerplate below...
	//
	// Location class can be found in location.hh and posistion.hh files. It's just a bit too much
	// boilerplate for this small example. Bummer.
%}

%option nodefault
%option noyywrap
%option c++
%option yyclass="Scanner"
%option prefix="spl_"

%%

[0-9]+ {return Parser::make_INT(atoi(yytext),spl::location());}
[0-9]+\.[0-9]+ {return Parser::make_FLOAT(atof(yytext),spl::location());}
[a-zA-Z_][a-zA-Z0-9_]* {return Parser::make_ID(yytext,spl::location());}
"int" {return Parser::make_TYPE("This is int",spl::location());}
"float" {return Parser::make_TYPE("This is float",spl::location());}
"char" {return Parser::make_TYPE("This is char",spl::location());}
"struct" {return Parser::make_STRUCT(spl::location());}
"if" {return Parser::make_IF(spl::location());}
"else" {return Parser::make_ELSE(spl::location());}
"while" {return Parser::make_WHILE(spl::location());}
"return" {return Parser::make_RETURN(spl::location());}
"." {return Parser::make_DOT(spl::location());}
";" {return Parser::make_SEMI(spl::location());}
"," {return Parser::make_COMMA(spl::location());}
"=" {return Parser::make_ASSIGN(spl::location());}
"<" {return Parser::make_LT(spl::location());}
"<=" {return Parser::make_LE(spl::location());}
">" {return Parser::make_GT(spl::location());}
">=" {return Parser::make_GE(spl::location());}
"!=" {return Parser::make_NE(spl::location());}
"==" {return Parser::make_EQ(spl::location());}
"+" {return Parser::make_PLUS(spl::location());}
"-" {return Parser::make_MINUS(spl::location());}
"*" {return Parser::make_MUL(spl::location());}
"/" {return Parser::make_DIV(spl::location());}
"&&" {return Parser::make_AND(spl::location());}
"||" {return Parser::make_OR(spl::location());}
"!" {return Parser::make_NOT(spl::location());}
"(" {return Parser::make_LP(spl::location());}
")" {return Parser::make_RP(spl::location());}
"[" {return Parser::make_LB(spl::location());}
"]" {return Parser::make_RB(spl::location());}
"{" {return Parser::make_LC(spl::location());}
"}" {return Parser::make_RC(spl::location());}

<<EOF>>     { return yyterminate();/*return Parser::make_TERMINATE(spl::location());*/ }


%%

int main(int argc, char **argv)
{
	spl::Scanner scanner;
	spl::Parser parser(scanner);
	parser.parse();
	return 0;
}
