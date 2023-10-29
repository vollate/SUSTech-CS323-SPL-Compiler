#ifndef Frontage_H
#define Frontage_H

#include "Parser.hpp"
#include "Scanner.hpp"

#include <cstdint>
#include <list>
#include <memory>
#include <optional>
#include <stack>
#include <type_traits>
#include <utility>
#include <variant>

namespace spl {
    struct ASTNode;
    using token_type = Parser::token_type;
    using variant_type = std::variant<int32_t, float, std::string>;
    class Frontage {
    public:
        Frontage(std::string const &filePath);

        bool parse();

        void clear();

        std::string str() const;
        spl::location location() const;
        std::string error()const;
        void appendError(std::string const& error);
    private:
        std::stack<ASTNode *> m_parentNodeStack;
        Scanner m_scanner;
        Parser m_parser;
        std::list<std::unique_ptr<ASTNode>> m_ast;
        std::list<std::string> m_errors;
        spl::location m_location;

//        void increaseLocation(int32_t loc);
        void increaseLine(int32_t line=1);

        // Used to get last Scanner location. Used in error messages.

        friend class Parser;

        friend class Scanner;
    };

}  // namespace spl

#endif  // Frontage_H
