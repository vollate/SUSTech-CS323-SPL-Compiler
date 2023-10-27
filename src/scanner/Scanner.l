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

	#define YY_USER_ACTION m_frontage.increaseLocation(yyleng);

	#define GENERATE_RETURN_TOKEN(__name,__type,__value)\
	return Parser::make_##__name(std::make_unique<ASTNode>(token_type::__type,__value),spl::location())
%}

%option nodefault
%option noyywrap
%option c++
%option yyclass="Scanner"
%option prefix="spl_"

space [ ]+
%%

[0-9]+ {GENERATE_RETURN_TOKEN(INT,INT,atoi(yytext));}
[0-9]+\.[0-9]+ {GENERATE_RETURN_TOKEN(FLOAT,FLOAT,std::strtof(yytext,nullptr));}

{space} {cout<<"space\n";}
<<EOF>>     { return Parser::make_END(spl::location());}
.|\n       { cout<<"error\n";GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"unknow token: "+std::string(yytext));}

%%
/*TODO @LHL
 * 学着上面的例子，把下面的token重写一下
 * 例如，对于struct my_struct: STRUCT ID
 *      struct的__name和__type都是STRUCT，__value是int,float,std::string中的任意值，建议直接填0(因为它本是就是一个固定的token)
 *      ID 则分别是ID,ID,"my_struct"
 * 待实现功能:
 * 处理行注释和块注释
 * 处理词法错误，加了一个叫LEX_ERROR的token，用来处理词法错误,如非法16进制数，非法浮点数等，超长整形数等，返回一个string说明发生了什么错误
[a-zA-Z_][a-zA-Z0-9_]* {return Parser::make_ID(yytext,spl::location());}
"int" {return Parser::make_TYPE("This is int",spl::location());}
"float" {return Parser::make_TYPE("This is float",spl::location());}
"char" {return Parser::make_TYPE("This is char",spl::location());}
"struct" {GENERATE_RETURN_TOKEN(STRUCT,STRUCT,"struct");}
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
*/