#ifndef Frontage_H
#define Frontage_H

#include "Parser.hpp"
#include "Scanner.hpp"
#include <cstdint>
#include <list>
#include <variant>

namespace spl {

using token_type = Parser::token_type;

class Command;

struct ASTNode {
  token_type type;
  std::variant<int32_t, float, std::string> value;
};

class Frontage {
 public:
  Frontage(std::string const& filePath);

  bool parse();

  void clear();

  std::string str() const;
  void append(ASTNode&& node);

  friend class Parser;

  friend class Scanner;

 private:
  Scanner m_scanner;
  Parser m_parser;
  std::list<ASTNode> m_ast;
  int32_t m_location;

  void increaseLocation(int32_t loc);

  // Used to get last Scanner location. Used in error messages.
  int32_t location() const;
};

}  // namespace spl

#endif  // Frontage_H
