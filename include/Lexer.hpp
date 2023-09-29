#pragma once

#include <string>
#ifndef yyFlexLexerOnce
#include <FlexLexer.h>
#endif

class Lexer : public yyFlexLexer {
  std::string parseResult;

public:
  virtual int yylex();
  std::string lexicalAnalysis(const char *filePath);
};
