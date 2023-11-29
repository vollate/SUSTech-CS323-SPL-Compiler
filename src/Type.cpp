#include "Type.hpp"
#include "Frontage.hpp"
#include "Parser.hpp"

#include <cmath>
#include <optional>
#include <string>
#include <unordered_map>

namespace spl {
    using std::string;

    bool isInt(const NodeType& node) {
        return static_cast<TOKEN_TYPE>(node->type) == TOKEN_TYPE::TYPE && std::get<string>(node->typeValue) == "int";
    }

    bool isFloat(const NodeType& node) {
        return static_cast<TOKEN_TYPE>(node->type) == TOKEN_TYPE::TYPE && std::get<string>(node->typeValue) == "float";
    }

    bool isChar(const NodeType& node) {
        return static_cast<TOKEN_TYPE>(node->type) == TOKEN_TYPE::TYPE && std::get<string>(node->typeValue) == "char";
    }

    std::optional<string> canAssign(const NodeType& lhs, const NodeType& rhs) {
        switch(static_cast<ValueType>(lhs->type)) {
            case ValueType::INT:
            case ValueType::FLOAT:
            case ValueType::CHAR:
                return SEMANTIC_ERROR_TEMPLATE[5];  // assign 2 a rvalue
            default:
                break;
        }
        return std::nullopt;
    }

    std::optional<string> declareVariable(DefTable& defTable, VarTable& varTable, const NodeType& specifier,
                                          const NodeType& varNode) {
        if(varTable.count(std::get<string>(varNode->value)) != 0) {
            return "02" + std::get<string>(varNode->value) + SEMANTIC_ERROR_TEMPLATE[2];  // redeclare error
        }
        splDebugLog("Variable Add, Name:  " + std::get<string>(varNode->value) +
                    "\t\tType: " + std::get<string>(specifier->value));
        switch(specifier->valueType) {
            case ValueType::TYPE:
                varTable[std::get<string>(varNode->value)] = { varNode->valueType, std::get<string>(varNode->typeValue), 1 };
                break;
            case ValueType::STRUCT:
                varTable[std::get<string>(varNode->value)] = { ValueType::STRUCT, std::get<string>(varNode->typeValue), 1 };
            default:
                break;
        }
        return std::nullopt;
    }
    std::optional<string> defineStruct(DefTable& table, const NodeType& structSpecifier) {
        return std::nullopt;
    }
}  // namespace spl
