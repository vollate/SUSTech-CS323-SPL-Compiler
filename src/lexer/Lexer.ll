%option c++
%option noyywrap
%option yyclass="Lexer"
%{
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include "Lexer.hpp"
%}

INT     "TODO"
FLOAT   "TODO"
CHAR    [a-zA-Z]
TYPE    int|float|char 
STRUCT  struct
IF      if
ELSE    else 
WHILE   while
RETURN  return
DOT     . 
SEMI    ; 
COMMA   , 
ASSIGN  =
LT      < 
LE      <=
GT      > 
GE      >=
NE      != 
EQ      ==
PLUS    + 
MINUS   - 
MUL     * 
DIV     / 
AND     &&
OR      || 
NOT     ! 
LP      (
RP      ) 
LB      [ 
RB      ]
LC      { 
RC      } 

%%
<<EOF>> {std::endl(std::cout<<"finish"); yyterminate();}
%%

std::string Lexer::lexicalAnalysis(const char *filePath) {
    std::fstream fileBuf(filePath,std::ios::in);
    if(!fileBuf.is_open()){
        throw std::runtime_error("Failed to open file");
    }
    yyin.rdbuf(fileBuf.rdbuf());
    parseResult.clear();
    yylex();
  return parseResult;
}
