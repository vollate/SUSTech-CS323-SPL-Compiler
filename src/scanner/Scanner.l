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
	#define GENERATE_RETURN_TOKEN(__name,__type,__type_value,__value_type,__value)\
{	cout<<"type: "<<#__name<<"\tvalue: "<<__type_value<<'\n';\
return Parser::make_##__name(std::make_unique<ParseNode>(TOKEN_TYPE::__type,__type_value,VALUE_TYPE::__value_type,__value,m_frontage.location()),m_frontage.location());}
#else
    #define GENERATE_RETURN_TOKEN(__name,__type,__type_value,__value_type,__value)\
    return Parser::make_##__name(std::make_unique<ParseNode>(TOKEN_TYPE::__type,__type_value,VALUE_TYPE::__value_type,__value,m_frontage.location()),m_frontage.location())
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
int32_oct 0[0-7]+
int32_hex_illgeal_char 0[xX]({digit_hex}*{invalid_hex_char}+{digit_hex}*)
float {digit}*\.{digit}*
char '([a-zA-Z0-9!@#$%^&*()_+{}\[\]:;<>,.?~]|\\[\'\"?\\abfnrtv0])'
char_hex '\\x(0|[1-9a-fA-F]{digit_hex})'
char_illegal '\\x{digit_hex}*[g-zG-Z]{digit_hex}*'
char_over_length '\\x{digit_hex}{3,}'
ID {letter_}({letter_}|{digit})*
invlid_ID [0-9]+{ID}
space [ ]+
include_s \#{space}*include{space}*<[^>]+>
include_u \#{space}*include{space}*\"[^\"]+\"
%%

{include_s} {
                std::string text(yytext);
                auto start=text.find('<');
                auto end=text.find('>',start+1);
                auto fileName=text.substr(start+1,end-start-1);
                if(m_frontage.sysFirstInclude(fileName)){
                    GENERATE_RETURN_TOKEN(INCLUDE,INCLUDE,std::string( "Include: " )+fileName,NONE,0);
                }else{
                    GENERATE_RETURN_TOKEN(OTHER_ERROR,OTHER_ERROR,std::string("Can not find header ")+fileName,NONE,0);
                }
    }
{include_u} {
                std::string text(yytext);
                auto start=text.find('"');
                auto end=text.find('"',start+1);
                auto fileName=text.substr(start+1,end-start-1);
               if(m_frontage.userFirstInclude(fileName)){
                    GENERATE_RETURN_TOKEN(INCLUDE,INCLUDE,std::string( "Include: " )+fileName,NONE,0);
                }else{
                    GENERATE_RETURN_TOKEN(OTHER_ERROR,OTHER_ERROR,std::string("Can not find header ")+fileName,NONE,0);
                }
    }

"\n" {m_frontage.increaseLine();}
[ \t\r]+ { /* Ignore whitespace */}
"//".*  { /* Ignore single line comments */ }
"/*"([^*]|\*[^/])*"*/" { /* Ignore multi line comments */ }

"int"    { GENERATE_RETURN_TOKEN(TYPE, TYPE, "int",NONE,0); }
"float"  { GENERATE_RETURN_TOKEN(TYPE, TYPE, "float",NONE,0); }
"char"   { GENERATE_RETURN_TOKEN(TYPE, TYPE, "char",NONE,0); }
"struct" { GENERATE_RETURN_TOKEN(STRUCT, STRUCT, "STRUCT",NONE,0); }
"if"     { GENERATE_RETURN_TOKEN(IF, IF, "IF",NONE,0); }
"else"   { GENERATE_RETURN_TOKEN(ELSE, ELSE, "ELSE",NONE,0); }
"while"  { GENERATE_RETURN_TOKEN(WHILE, WHILE, "WHILE",NONE,0); }
"return" { GENERATE_RETURN_TOKEN(RETURN, RETURN, "RETURN",NONE,0); }

\.       { GENERATE_RETURN_TOKEN(DOT, DOT, "DOT",NONE,0); }
";"      { GENERATE_RETURN_TOKEN(SEMI, SEMI, "SEMI",NONE,0); }
","      { GENERATE_RETURN_TOKEN(COMMA, COMMA, "COMMA",NONE,0); }
"="      { GENERATE_RETURN_TOKEN(ASSIGN, ASSIGN, "ASSIGN",NONE,0); }
"<"      { GENERATE_RETURN_TOKEN(LT, LT, "LT",NONE,0); }
"+="     { GENERATE_RETURN_TOKEN(PLUS_EQUAL,PLUS_EQUAL,"PLUS_EQUAL",NONE,0); }
"-="     { GENERATE_RETURN_TOKEN(MINUS_EQUAL,MINUS_EQUAL,"MINUS_EQUAL",NONE,0); }
"*="     { GENERATE_RETURN_TOKEN(MUL_EQUAL,MUL_EQUAL,"MUL_EQUAL",NONE,0); }
"/="     { GENERATE_RETURN_TOKEN(DIV_EQUAL,DIV_EQUAL,"DIV_EQUAL",NONE,0); }
"<="     { GENERATE_RETURN_TOKEN(LE, LE, "LE",NONE,0); }
">"      { GENERATE_RETURN_TOKEN(GT, GT, "GT",NONE,0); }
">="     { GENERATE_RETURN_TOKEN(GE, GE, "GE",NONE,0); }
"!="     { GENERATE_RETURN_TOKEN(NE, NE, "NE",NONE,0); }
"=="     { GENERATE_RETURN_TOKEN(EQ, EQ, "EQ",NONE,0); }
"+"      { GENERATE_RETURN_TOKEN(PLUS, PLUS, "PLUS",NONE,0); }
"-"      { GENERATE_RETURN_TOKEN(MINUS, MINUS, "MINUS",NONE,0); }
"*"      { GENERATE_RETURN_TOKEN(MUL, MUL, "MUL",NONE,0); }
"/"      { GENERATE_RETURN_TOKEN(DIV, DIV, "DIV",NONE,0); }
"&&"     { GENERATE_RETURN_TOKEN(AND, AND, "AND",NONE,0); }
"||"     { GENERATE_RETURN_TOKEN(OR, OR, "OR",NONE,0); }
"!"      { GENERATE_RETURN_TOKEN(NOT, NOT, "NOT",NONE,0); }
"("      { GENERATE_RETURN_TOKEN(LP, LP, "LP",NONE,0); }
")"      { GENERATE_RETURN_TOKEN(RP, RP, "RP",NONE,0); }
"["      { GENERATE_RETURN_TOKEN(LB, LB, "LB",NONE,0); }
"]"      { GENERATE_RETURN_TOKEN(RB, RB, "RB",NONE,0); }
"{"      { GENERATE_RETURN_TOKEN(LC, LC, "LC",NONE,0); }
"}"      { GENERATE_RETURN_TOKEN(RC, RC, "RC",NONE,0); }

{ID}     { GENERATE_RETURN_TOKEN(ID, ID, std::string(yytext),NONE,0); }

{int32_dec} { int32_t val=atoi(yytext); GENERATE_RETURN_TOKEN(INT, INT, val,INT,val); }
{int32_hex} { int32_t val=static_cast<int32_t>(strtol(yytext, nullptr, 0)); GENERATE_RETURN_TOKEN(INT, INT, val, INT, val); }
{int32_oct} { int32_t val=static_cast<int32_t>(stoi(yytext, 0, 8)); GENERATE_RETURN_TOKEN(INT, INT, val, INT, val); }
{int32_hex_illgeal_char} {GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"Invalid int32: "+std::string(yytext),NONE,0);}
{float}     { std::string s(yytext);
              if(s.length()==1 && s[0]=='.'){
                GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"unknown token: "+s,NONE,0);
              }
              float val=static_cast<float>(atof(yytext));
              GENERATE_RETURN_TOKEN(FLOAT, FLOAT, val, FLOAT, val);
            }
{char}      { std::string val=std::string(yytext); GENERATE_RETURN_TOKEN(CHAR, CHAR, val, CHAR, val); }
{char_hex}  { std::string val=std::string(yytext); GENERATE_RETURN_TOKEN(CHAR, CHAR, val, CHAR, val); }
{char_illegal} {GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"Illegal char: "+std::string(yytext),NONE,0);}
{char_over_length} {GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"Over-length char: "+std::string(yytext),NONE,0);}
{invlid_ID} {GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"Invalid ID: "+std::string(yytext),NONE,0);}
<<EOF>>     { return Parser::make_END(spl::location());}
.           { GENERATE_RETURN_TOKEN(LEXICAL_ERROR,LEXICAL_ERROR,"unknown lexeme: "+std::string(yytext),NONE,0);}
%%
