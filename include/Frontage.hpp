#ifndef Frontage_H
#define Frontage_H

#include "Parser.hpp"
#include "Scanner.hpp"
#include "Semantic.hpp"

#include <cstdint>
#include <list>
#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace spl {
    struct ParseNode;
    using TOKEN_TYPE = Parser::token_type;
    using variant_type = std::variant<int32_t, float, std::string>;

    class Frontage {

    public:
        std::vector<std::string> SysIncludePath;

        std::optional<std::string> findHeaderSys(const std::string& headerName);

        std::optional<std::string> findHeaderUser(const std::string& headerName, const std::string filePath);

        Frontage(std::string const& filePath);

        bool parse();
        bool semantic();

        void clear();

        std::string parseTree() const;

        spl::location location() const;

        std::string syntaxError() const;

        std::string semanticError() const;

        void appendSyntaxError(std::string const& error);

    private:
        std::list<std::unique_ptr<Frontage>> m_includeTree;
        Scanner m_scanner;
        Parser m_parser;
        SemanticAnalyzer m_semanticAnalysizer;
        std::vector<std::unique_ptr<ParseNode>> m_parseTree;
        std::vector<std::string> m_errors;
        spl::location m_location;

        bool sysFirstInclude(const std::string& name);

        bool userFirstInclude(const std::string& name);

        void increaseLine(int32_t line = 1);

        friend class Parser;

        friend class Scanner;

        friend class SemanticAnalyzer;
    };

    class SubFrontage : public Frontage {
    public:
        SubFrontage(std::string const& filePath);

        friend class Parser;

        friend class Scanner;
    };
}  // namespace spl

#endif  // Frontage_H
