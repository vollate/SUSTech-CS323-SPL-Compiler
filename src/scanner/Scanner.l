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

	//#define YY_USER_ACTION m_frontage.increaseLocation(yyleng);

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
int32_hex_illgeal_char 0[xX]({digit_hex}*{invalid_hex_char}+{digit_hex}*)
float {digit}*\.{digit}*
char '([a-zA-Z0-9!@#$%^&*()_+{}\[\]:;<>,.?~]|\\[\'\"?\\abfnrtv0])'
char_hex '\\x(0|[1-9a-fA-F]{digit_hex})'
char_illegal '\\x([1-9a-fA-F]){digit_hex}*[g-z]{digit_hex}*'
char_over_length '\\x{digit_hex}{3,}'
ID {letter_}({letter_}|{digit})*
invlid_ID [0-9]+{ID}
space [ ]+
%%

"\n" {m_frontage.increaseLine();}
[ \t\r]+ { /* Ignore whitespace */}
"//".*  { /* Ignore single line comments */ }
"/*"([^*]|\*[^/])*"*/" { /* Ignore multi line comments */ }
"int"    { GENERATE_RETURN_TOKEN(TYPE, TYPE, "int"); }
"float"  { GENERATE_RETURN_TOKEN(TYPE, TYPE, "float"); }
"char"   { GENERATE_RETURN_TOKEN(TYPE, TYPE, "char"); }
"struct" { GENERATE_RETURN_TOKEN(STRUCT, STRUCT, "STRUCT"); }
"if"     { GENERATE_RETURN_TOKEN(IF, IF, "IF"); }
"else"   { GENERATE_RETURN_TOKEN(ELSE, ELSE, "ELSE"); }
"while"  { GENERATE_RETURN_TOKEN(WHILE, WHILE, "WHILE"); }
"return" { GENERATE_RETURN_TOKEN(RETURN, RETURN, "RETURN"); }

\.      { GENERATE_RETURN_TOKEN(DOT, DOT, "DOT"); }
";"      { GENERATE_RETURN_TOKEN(SEMI, SEMI, "SEMI"); }
","      { GENERATE_RETURN_TOKEN(COMMA, COMMA, "COMMA"); }
"="      { GENERATE_RETURN_TOKEN(ASSIGN, ASSIGN, "ASSIGN"); }
"<"      { GENERATE_RETURN_TOKEN(LT, LT, "LT"); }
"<="     { GENERATE_RETURN_TOKEN(LE, LE, "LE"); }
">"      { GENERATE_RETURN_TOKEN(GT, GT, "GT"); }
">="     { GENERATE_RETURN_TOKEN(GE, GE, "GE"); }
"!="     { GENERATE_RETURN_TOKEN(NE, NE, "NE"); }
"=="     { GENERATE_RETURN_TOKEN(EQ, EQ, "EQ"); }
"+"      { GENERATE_RETURN_TOKEN(PLUS, PLUS, "PLUS"); }
"-"      { GENERATE_RETURN_TOKEN(MINUS, MINUS, "MINUS"); }
"*"      { GENERATE_RETURN_TOKEN(MUL, MUL, "MUL"); }
"/"      { GENERATE_RETURN_TOKEN(DIV, DIV, "DIV"); }
"&&"     { GENERATE_RETURN_TOKEN(AND, AND, "AND"); }
"||"     { GENERATE_RETURN_TOKEN(OR, OR, "OR"); }
"!"      { GENERATE_RETURN_TOKEN(NOT, NOT, "NOT"); }
"("      { GENERATE_RETURN_TOKEN(LP, LP, "LP"); }
")"      { GENERATE_RETURN_TOKEN(RP, RP, "RP"); }
"["      { GENERATE_RETURN_TOKEN(LB, LB, "LB"); }
"]"      { GENERATE_RETURN_TOKEN(RB, RB, "RB"); }
"{"      { GENERATE_RETURN_TOKEN(LC, LC, "LC"); }
"}"      { GENERATE_RETURN_TOKEN(RC, RC, "RC"); }

{ID}     { GENERATE_RETURN_TOKEN(ID, ID, std::string(yytext)); }

{int32_dec} { GENERATE_RETURN_TOKEN(INT, INT, atoi(yytext)); }
{int32_hex} { GENERATE_RETURN_TOKEN(INT, INT, static_cast<int32_t>(strtol(yytext, nullptr, 0))); }
{int32_hex_illgeal_char} {GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"Invalid int32: "+std::string(yytext));}
{float}     { std::string s(yytext);
              if(s.length()==1 && s[0]=='.'){
                GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"unknown token: "+s);
              }
              GENERATE_RETURN_TOKEN(FLOAT, FLOAT, static_cast<float>(atof(yytext)));
            }
{char}      { GENERATE_RETURN_TOKEN(CHAR, CHAR, std::string(yytext)); }
{char_hex}  { GENERATE_RETURN_TOKEN(CHAR, CHAR, std::string(yytext)); }
{char_illegal} {GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"Illegal char: "+std::string(yytext));}
{char_over_length} {GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"Over-length char: "+std::string(yytext));}
{invlid_ID} {GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"Invalid ID: "+std::string(yytext));}
<<EOF>>     { return Parser::make_END(spl::location());}
.           { GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"unknown lexeme: "+std::string(yytext));}
%%
