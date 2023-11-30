#include "Semantic.hpp"
#include "Frontage.hpp"
#include "Parser.hpp"

#include <cmath>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

namespace spl {
    using std::string;
    void SemanticAnalysizer::appendError(int errorId, const location& location, std::string& msg) {
        std::stringstream ss;
        ss << "Error type " << errorId << " at Line " << location.end.line << ": " << msg;
        m_errors.push_back(ss.str());
    }

    bool SemanticAnalysizer::isInt(const NodeType& node) {
        return static_cast<TOKEN_TYPE>(node->type) == TOKEN_TYPE::TYPE && std::get<string>(node->typeValue) == "int";
    }

    bool SemanticAnalysizer::isFloat(const NodeType& node) {
        return static_cast<TOKEN_TYPE>(node->type) == TOKEN_TYPE::TYPE && std::get<string>(node->typeValue) == "float";
    }

    bool SemanticAnalysizer::isChar(const NodeType& node) {
        return static_cast<TOKEN_TYPE>(node->type) == TOKEN_TYPE::TYPE && std::get<string>(node->typeValue) == "char";
    }

    bool SemanticAnalysizer::canAssign(const NodeType& lhs, const NodeType& rhs) {
        switch(static_cast<ValueType>(lhs->type)) {
            case ValueType::INT:
            case ValueType::FLOAT:
            case ValueType::CHAR:
                return SEMANTIC_ERROR_TEMPLATE[5];  // assign 2 a rvalue
            default:
                break;
        }
        return true;
    }

    bool SemanticAnalysizer::declareVariable(const NodeType& specifier, const NodeType& varNode) {
        auto& varTable = m_varTables.back();
        if(varTable.count(std::get<string>(varNode->value)) != 0) {
            return false;
            // return "02" + std::get<string>(varNode->value) + SEMANTIC_ERROR_TEMPLATE[2];  // redeclare error
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
        return true;
    }

    bool SemanticAnalysizer::defineStruct(const NodeType& structSpecifier) {
        return true;
    }

    // const auto* baseNode = &$2;
    // while(true) {
    // const auto& subNodes = (*baseNode)->subNodes;
    // if(auto res=declareVariable(frontage.m_defTables.back(),frontage.m_varTables.back(),$1,subNodes.front()))
    // yyerror(res.value(), @$, ERROR_TYPE::SEMANTIC_ERROR, frontage);
    // if(subNodes.size() == 1) {
    // break;
    //} else {
    // baseNode = &subNodes.back();
    //}
    //}

    string SemanticAnalysizer::getErrors() const {
        std::stringstream ss;
        for(const auto& error : m_errors) {
            ss << error << '\n';
        }
        return ss.str();
    }

    bool SemanticAnalysizer::analyze() {
        m_varTables.clear();
        m_defTables.clear();

        return m_errors.empty();
    }
}  // namespace spl
