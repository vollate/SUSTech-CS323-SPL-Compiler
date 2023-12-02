#ifndef SPL_TYPE_H

#define SPL_TYPE_H

#include "Parser.hpp"

#include <any>
#include <array>
#include <cstdint>
#include <list>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace spl {
    inline const std::array<const char*, 20> SEMANTIC_ERROR_TEMPLATE = { " is used without a definition",
                                                                         " is invoked without a definition",
                                                                         " is redefined in the same scope",
                                                                         " is redefined",
                                                                         "unmatching type on both sides of assignment",
                                                                         "rvalue appears on the left-side of assignment",
                                                                         "unmatching operands",
                                                                         "incompatiable return type",
                                                                         "invalid argument number, except ",
                                                                         "indexing on non-array variable",
                                                                         "invoking non-function variable",
                                                                         "indexing by non-integer",
                                                                         "accessing with non-struct variable",
                                                                         "accessing an undefined structure member",
                                                                         "redefine the same structure type",
                                                                         "use struct without declare" };

    struct AryDef {
        ValueType type;
        size_t size;
        std::list<int> subAryLength;
    };

    struct FunDef {
        std::vector<ValueType> argTypes;
        std::vector<std::string> argIds;
        std::vector<std::pair<int, AryDef>> argAry;
        ValueType returnType;
        std::string returnTypeValue;
    };

    struct StructDef {
        std::vector<ValueType> memberTypes;
        std::vector<std::string> memberIds;
    };

    struct DefNode {
        ValueType type;
        std::variant<FunDef, AryDef, StructDef> val;
    };

    struct VarNode {
        ValueType type;
        std::string id;
        std::any val;
    };

    using SymbolTableble = std::unordered_map<std::string, DefNode>;
    using VarTable = std::unordered_map<std::string, VarNode>;

    class SemanticAnalyzer {

        Frontage& frontage;

        std::list<SymbolTableble> m_symbolTables;
        std::list<VarTable> m_varTables;
        std::vector<std::string> m_errors;
        std::vector<std::pair<std::string, FunDef>> waitToken;

        void appendError(int errorId, const location& location, const std::string& msg);

        bool dealDec(const NodeType& specifier, NodeType& dec);

        ValueType dealExp(NodeType& exp);

        bool dealVarDec(const NodeType& specifier, NodeType& varDec);

        bool dealDecList(NodeType& specifier, NodeType& decList);

        void dealExtDef(NodeType& extDef);

        bool dealArgs(NodeType& args);

        void dealSpecifier(NodeType& specifier);

        void dealStructSpecifier(NodeType& structSpecifier);

        std::optional<FunDef> dealFunDec(NodeType& specifier, NodeType& funDec);

        bool dealDefList(NodeType& defList);

        bool dealDef(NodeType& def);

        bool dealDefList(const std::string& structId, NodeType& defList);

        bool declareVariable(const NodeType& specifier, const NodeType& var);

        bool dealParamDec(NodeType& paramDec, FunDef& funDef);

        bool dealVarList(NodeType& varList, FunDef& funDef);

        bool dealStmtList(const FunDef& funDef, NodeType& stmtList);

        void dealExtDefList(NodeType& extDefList);

        bool dealCompSt(const FunDef& funDef, NodeType& compSt);

        SymbolTableble& getSymbolTable();

        bool hadDefined(const std::string& id, ValueType type);

        bool hadDeclareInCurrentScope(const std::string& id);

        bool hadDeclareInAllScope(const std::string& id);

    public:
        explicit SemanticAnalyzer(Frontage& frontage) : frontage(frontage) {}

        bool analyze();

        [[nodiscard]] std::string getErrors() const;
    };

}  // namespace spl

#endif
