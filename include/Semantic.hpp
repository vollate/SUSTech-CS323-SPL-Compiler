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
    inline const std::array<const std::string, 20> SEMANTIC_ERROR_TEMPLATE = {
        " is used without a definition",
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
        "use struct without declare",
        "unmatching type on both sides of logic operation",
        "unmatching type on both sides of compare operation",
        "unmatch type for function arguments"
    };

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
        std::string returnTypeName;
    };

    struct StructDef {
        std::vector<ValueType> memberTypes;
        std::vector<std::string> memberIds;
        std::vector<std::pair<int, AryDef>> memberAry;
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

    using SymbolTableble = std::unordered_map<std::string, std::list<DefNode>>;
    using VarTable = std::unordered_map<std::string, VarNode>;

    class SemanticAnalyzer {

        Frontage& frontage;

        std::list<SymbolTableble> m_symbolTables;
        std::list<VarTable> m_varTables;
        std::vector<std::string> m_errors;
        std::vector<std::pair<std::string, FunDef>> waitFun;

        std::optional<VarNode> getVarNode(const std::string& id) const;

        bool processArray(AryDef& aryDef, NodeType& exp);

        std::optional<ValueType> processArrayExp(NodeType& exp);

        bool processArgs(FunDef& funDef, NodeType& args);

        void appendError(int errorId, const location& location, const std::string& msg);

        bool processStmt(const FunDef& funDef, NodeType& stmt);

        bool canAssign(const NodeType& lhs, const NodeType& rhs);

        bool processDec(NodeType& specifier, NodeType& dec);

        bool processDec(StructDef& structDef, NodeType& specifier, NodeType& dec);

        void processExp(NodeType& exp);

        bool processVarDec(NodeType& specifier, NodeType& varDec, bool structDef = false);

        bool processDecList(NodeType& specifier, NodeType& decList);

        bool processDecList(StructDef& structDef, NodeType& specifier, NodeType& decList);

        void processExtDef(NodeType& extDef);

        bool processArgs(NodeType& args);

        bool processSpecifier(NodeType& specifier);

        bool processStructSpecifier(NodeType& structSpecifier);

        std::optional<FunDef> processFunDec(NodeType& specifier, NodeType& funDec);

        bool processDefList(NodeType& defList);

        bool processDefList(StructDef& structDef, NodeType& defList);

        bool processDef(StructDef& structDef, NodeType& defList);

        bool processDef(NodeType& def);

        bool processParamDec(NodeType& paramDec, FunDef& funDef);

        bool processVarList(NodeType& varList, FunDef& funDef);

        bool processStmtList(const FunDef& funDef, NodeType& stmtList);

        void processExtDefList(NodeType& extDefList);

        bool processCompSt(const FunDef& funDef, NodeType& compSt);

        bool declareVariable(NodeType& specifier, const NodeType& var);

        SymbolTableble& getSymbolTable();

        std::optional<DefNode> getDefNode(const std::string& id, ValueType type);

        void insertSymbolTable(const std::string& id, const DefNode& defNode);

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
