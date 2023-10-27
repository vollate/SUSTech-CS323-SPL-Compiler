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

    private:
        std::stack<ASTNode *> m_parentNodeStack;
        Scanner m_scanner;
        Parser m_parser;
        std::list<std::unique_ptr<ASTNode>> m_ast;
        int32_t m_location;

        void increaseLocation(int32_t loc);

        void pushParent(ASTNode *node = nullptr);

        void emptyParentStack();

        ASTNode *selectParent(const std::string &nonTerminalName);

        ASTNode *getParent();

        template<typename F, typename... P>
        void reduceToNonTerminal(const std::string &nonTerminalParent, F &&v0, P &&... v) {
            auto parent = selectParent(nonTerminalParent);
            pushParent(parent);
            recursiveReduction(parent, std::forward<F>(v0), std::forward<P>(v)...);
        }

        template<typename F, typename... P>
        void recursiveReduction(ASTNode *parent, F &&v0, P &&... v) {
            parent->subNodes.push_back(make_unique<ASTNode>(token_type::NON_TERMINAL, v0));
            if constexpr (sizeof...(P)) {
                recursiveReduction(parent, std::forward<P>(v)...);
            }
        }

        // Used to get last Scanner location. Used in error messages.
        int32_t location() const;

        friend class Parser;

        friend class Scanner;
    };

}  // namespace spl

#endif  // Frontage_H
