#include "Type.hpp"
#include "Frontage.hpp"
#include "Parser.hpp"

namespace spl {

    bool isInt(const NodeType& node) {
        return static_cast<TOKEN_TYPE>(node->type) == TOKEN_TYPE::TYPE && std::get<std::string>(node->typeValue) == "int";
    }

    bool isFloat(const NodeType& node) {
        return static_cast<TOKEN_TYPE>(node->type) == TOKEN_TYPE::TYPE && std::get<std::string>(node->typeValue) == "float";
    }

    bool isChar(const NodeType& node) {
        return static_cast<TOKEN_TYPE>(node->type) == TOKEN_TYPE::TYPE && std::get<std::string>(node->typeValue) == "char";
    }

    std::optional<std::string> canAssign(const NodeType& lhs, const NodeType& rhs) {
        switch(static_cast<VALUE_TYPE>(lhs->type)) {
            case VALUE_TYPE::INT:
            case VALUE_TYPE::FLOAT:
            case VALUE_TYPE::CHAR:
                return SEMANTIC_ERROR_TEMPLATE[5];  // assign 2 a rvalue
            default:
                break;
        }
        return std::nullopt;
    }
}  // namespace spl
