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

#ifdef SPL_DEBUG
	#define GENERATE_RETURN_TOKEN(__name,__type,__value)\
{	cout<<"type: "<<#__name<<"\tvalue: "<<__value<<'\n';\
return Parser::make_##__name(std::make_unique<ASTNode>(token_type::__type,m_frontage.location(),__value),m_frontage.location());}
#else
    #define GENERATE_RETURN_TOKEN(__name,__type,__value)\
    return Parser::make_##__name(std::make_unique<ASTNode>(token_type::__type,m_frontage.location(),__value),m_frontage.location())
#endif
%}

%option nodefault
%option yylineno
%option noyywrap
%option c++
%option yyclass="Scanner"
%option prefix="spl_"

letter [A-Za-z]
letter_ [A-Za-z_]
digit [0-9]
digit_hex [0-9a-fA-F]
invalid_hex_char [g-zG-Z]
int32_dec (0|([1-9]{digit}*))
int32_hex 0[xX]({digit_hex}+)
int32_hex_error_illgeal_char 0[xX]({digit_hex}*{invalid_hex_char}+{digit_hex}*)

float {digit}*\.{digit}*
ID {letter_}({letter_}|{digit})*
space [ ]+
%%

"\n" {m_frontage.increaseLine();cout<<"newline\n";}
[ \t\r]+ { /* Ignore whitespace */}
"//".*  { /* Ignore single line comments */ }
"/*"([^*]|\*[^/])*"*/" { /* Ignore multi line comments */ }
"int"    { GENERATE_RETURN_TOKEN(TYPE, TYPE, "int"); }
"float"  { GENERATE_RETURN_TOKEN(TYPE, TYPE, "float"); }
"char"   { GENERATE_RETURN_TOKEN(TYPE, TYPE, "char"); }
"struct" { GENERATE_RETURN_TOKEN(STRUCT, STRUCT, 0); }
"if"     { GENERATE_RETURN_TOKEN(IF, IF, 0); }
"else"   { GENERATE_RETURN_TOKEN(ELSE, ELSE, 0); }
"while"  { GENERATE_RETURN_TOKEN(WHILE, WHILE, 0); }
"return" { GENERATE_RETURN_TOKEN(RETURN, RETURN, 0); }

\.      { GENERATE_RETURN_TOKEN(DOT, DOT, 0); }
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

{ID}     { GENERATE_RETURN_TOKEN(ID, ID, std::string(yytext)); }
{int32_dec} { GENERATE_RETURN_TOKEN(INT, INT, atoi(yytext)); }
{int32_hex} { GENERATE_RETURN_TOKEN(INT, INT, static_cast<int32_t>(strtol(yytext, nullptr, 0))); }
{float}     { std::string s(yytext);
              if(s.length()==1 && s[0]=='.'){
                GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"unknown token: "+s);
              }
              GENERATE_RETURN_TOKEN(FLOAT, FLOAT, static_cast<float>(atof(yytext))); }

<<EOF>>     { cout<<"end\n";return Parser::make_END(spl::location());}
.           {}
%%
