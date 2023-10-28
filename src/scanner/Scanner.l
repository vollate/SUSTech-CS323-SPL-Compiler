%{
	#include <iostream>
	#include <cstdlib>
	#include <cerrno>
	#include <climits>
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

[0-9]+(\.[0-9]+)?([eE][+-]?[0-9]+)? {
    char* endptr;
    float val = std::strtof(yytext, &endptr);

    if (endptr != yytext + strlen(yytext) || errno == ERANGE) {
        GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"Invalid float: "+std::string(yytext));
    } else {
        GENERATE_RETURN_TOKEN(FLOAT,FLOAT,val);
    }
}

0[xX][0-9a-fA-F]*[^0-9a-fA-F]+|[0-9a-fA-F]+ {
    if (strspn(yytext + 2, "0123456789abcdefABCDEF") != strlen(yytext) - 2) {
        GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"Invalid hexadecimal: "+std::string(yytext));
    } else {
        GENERATE_RETURN_TOKEN(INT,INT,(int)std::strtol(yytext,nullptr,16));
    }
}

0[0-7]*[^0-7]+|[0-7]+ {
	if (strspn(yytext + 1, "01234567") != strlen(yytext) - 1) {
		GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"Invalid octal: "+std::string(yytext));
	} else {
		GENERATE_RETURN_TOKEN(INT,INT,(int)std::strtol(yytext,nullptr,8));
	}
}

[1-9][0-9]* {
	char* endptr;
	int64_t val = std::strtol(yytext, &endptr, 10);

	if (endptr != yytext + strlen(yytext) || errno == ERANGE) {
		GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"Invalid integer: "+std::string(yytext));
	} else if(val > INT_MAX || val < INT_MIN) {
		GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"Integer out of range: "+std::string(yytext));
	} else {
		GENERATE_RETURN_TOKEN(INT,INT,(int)val);
	}
}

"int"    { GENERATE_RETURN_TOKEN(TYPE, TYPE, 0); }
"float"  { GENERATE_RETURN_TOKEN(TYPE, TYPE, 0); }
"char"   { GENERATE_RETURN_TOKEN(TYPE, TYPE, 0); }
"struct" { GENERATE_RETURN_TOKEN(STRUCT, STRUCT, 0); }
"if"     { GENERATE_RETURN_TOKEN(IF, IF, 0); }
"else"   { GENERATE_RETURN_TOKEN(ELSE, ELSE, 0); }
"while"  { GENERATE_RETURN_TOKEN(WHILE, WHILE, 0); }
"return" { GENERATE_RETURN_TOKEN(RETURN, RETURN, 0); }
[a-zA-Z_][a-zA-Z0-9_]* { GENERATE_RETURN_TOKEN(ID, ID, std::string(yytext)); }
[a-zA-Z_0-9]+[^a-zA-Z0-9_ \t\n]+ { GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"Invalid ID: "+std::string(yytext)); }

"."      { GENERATE_RETURN_TOKEN(DOT, DOT, 0); }
";"      { GENERATE_RETURN_TOKEN(SEMI, SEMI, 0); }
","      { GENERATE_RETURN_TOKEN(COMMA, COMMA, 0); }
"="      { GENERATE_RETURN_TOKEN(ASSIGN, ASSIGN, 0); }
"<"      { GENERATE_RETURN_TOKEN(LT, LT, 0); }
"<="     { GENERATE_RETURN_TOKEN(LE, LE, 0); }
">"      { GENERATE_RETURN_TOKEN(GT, GT, 0); }
">="     { GENERATE_RETURN_TOKEN(GE, GE, 0); }
"!="     { GENERATE_RETURN_TOKEN(NE, NE, 0); }
"=="     { GENERATE_RETURN_TOKEN(EQ, EQ, 0); }
"+"      { GENERATE_RETURN_TOKEN(PLUS, PLUS, 0); }
"-"      { GENERATE_RETURN_TOKEN(MINUS, MINUS, 0); }
"*"      { GENERATE_RETURN_TOKEN(MUL, MUL, 0); }
"/"      { GENERATE_RETURN_TOKEN(DIV, DIV, 0); }
"&&"     { GENERATE_RETURN_TOKEN(AND, AND, 0); }
"||"     { GENERATE_RETURN_TOKEN(OR, OR, 0); }
"!"      { GENERATE_RETURN_TOKEN(NOT, NOT, 0); }
"("      { GENERATE_RETURN_TOKEN(LP, LP, 0); }
")"      { GENERATE_RETURN_TOKEN(RP, RP, 0); }
"["      { GENERATE_RETURN_TOKEN(LB, LB, 0); }
"]"      { GENERATE_RETURN_TOKEN(RB, RB, 0); }
"{"      { GENERATE_RETURN_TOKEN(LC, LC, 0); }
"}"      { GENERATE_RETURN_TOKEN(RC, RC, 0); }

"//".*  { /* Ignore single line comments */ }

"/*"([^*]|\*[^/])*"*/" { /* Ignore multi line comments */ }

{space} {cout<<"space\n";}
<<EOF>>     { return Parser::make_END(spl::location());}
[^ \t\n\r]+|\n       { cout<<"error\n";GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"unknow token: "+std::string(yytext));}

%%
/*TODO @LHL
 * 学着上面的例子，把下面的token重写一下
 * 例如，对于struct my_struct: STRUCT ID
 *      struct的__name和__type都是STRUCT，__value是int,float,std::string中的任意值，建议直接填0(因为它本是就是一个固定的token)
 *      ID 则分别是ID,ID,"my_struct"
 * 待实现功能:
 * 处理行注释和块注释
 * 处理词法错误，加了一个叫LEX_ERROR的token，用来处理词法错误,如非法16进制数，非法浮点数等，超长整形数等，返回一个string说明发生了什么错误
*/
