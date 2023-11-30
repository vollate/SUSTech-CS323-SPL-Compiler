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
    inline const std::array<const char*, 15> SEMANTIC_ERROR_TEMPLATE = { " is used without a definition",
                                                                         " is invoked without a definition",
                                                                         " is redefined in the same scope",
                                                                         " is redefined",
                                                                         "unmatching type on both sides of assignment",
                                                                         "rvalue appears on the left-side of assignment",
                                                                         "",
                                                                         "",
                                                                         "",
                                                                         "",
                                                                         "",
                                                                         "",
                                                                         "",
                                                                         "",
                                                                         "" };

    struct FuncDef {
        std::vector<ValueType> argTypes;
        std::vector<std::string> argIds;
        ValueType retType;
    };

    struct AryDef {
        ValueType subType;
        size_t size;
        std::unique_ptr<AryDef> subAry;
    };

    struct StructDef {
        std::vector<ValueType> memberTypes;
        std::vector<std::string> memberIds;
    };

    struct DefNode {
        ValueType type;
        std::string name;
        std::variant<FuncDef, AryDef, StructDef> val;
    };

    struct VarNode {
        ValueType type;
        std::string id;
        std::any val;
    };

    using DefTable = std::unordered_map<std::string, DefNode>;
    using VarTable = std::unordered_map<std::string, VarNode>;

    class SemanticAnalysizer {

        Frontage& frontage;

        std::list<DefTable> m_defTables;
        std::list<VarTable> m_varTables;
        std::vector<std::string> m_errors;

        void appendError(int errorId, const location& location, std::string& msg);

        bool isInt(const NodeType& node);

        bool isFloat(const NodeType& node);

        bool isChar(const NodeType& node);

        bool canAssign(const NodeType& lhs, const NodeType& rhs);

        bool declareVariable(const NodeType& specifier, const NodeType& node);

        bool defineStruct(const NodeType& structSpecifier);

    public:
        SemanticAnalysizer(Frontage& frontage) : frontage(frontage) {}
        bool analyze();
        std::string getErrors() const;
    };

}  // namespace spl

#endif
