#include "Semantic.hpp"
#include "Frontage.hpp"
#include "Parser.hpp"

#include <algorithm>
#include <cmath>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>

namespace spl {
    using std::string;
    namespace {
        ValueType string2ValueType(const variant_type& var) {
            auto str = std::get<string>(var);
            if(str == "int") {
                return ValueType::INT;
            } else if(str == "float") {
                return ValueType::FLOAT;
            } else if(str == "char") {
                return ValueType::CHAR;
            }
            return ValueType::NONE;
        }

        bool isInt(const NodeType& node) {
            return static_cast<TOKEN_TYPE>(node->type) == TOKEN_TYPE::TYPE && std::get<string>(node->typeValue) == "int";
        }

        bool isFloat(const NodeType& node) {
            return static_cast<TOKEN_TYPE>(node->type) == TOKEN_TYPE::TYPE && std::get<string>(node->typeValue) == "float";
        }

        bool isChar(const NodeType& node) {
            return static_cast<TOKEN_TYPE>(node->type) == TOKEN_TYPE::TYPE && std::get<string>(node->typeValue) == "char";
        }

        bool dealArray(AryDef& aryDef, NodeType& varDec) {
            auto* varDecNode = &varDec;
            while(true) {
                if((*varDecNode)->subNodes.size() == 1) {
                    break;
                }
                int length = std::get<int>((*varDecNode)->subNodes[2]->value);
                if(length < 0) {
                    return false;
                }
                aryDef.subAryLength.push_front(length);
                varDecNode = &(*varDecNode)->subNodes[0];
            }
            return true;
        }
    }  // namespace

    void SemanticAnalyzer::appendError(int errorId, const location& location, const std::string& msg) {
        std::stringstream ss;
        ss << "Error type " << errorId << " at Line " << location.end.line << ": " << msg;
#ifdef SPL_DEBUG
        cerr << ss.str() << '\n';
#endif
        m_errors.push_back(ss.str());
    }

    bool SemanticAnalyzer::dealVarList(NodeType& varList, FunDef& funDef) {
        auto* varNode = &varList;
        m_varTables.emplace_back();
        while(true) {
            if(!dealParamDec((*varNode)->subNodes[0], funDef)) {
                m_varTables.pop_back();
                return false;
            } else if((*varNode)->subNodes.size() == 1) {
                break;
            }
            varNode = &(*varNode)->subNodes[2];
        }
        return true;
    }

    bool SemanticAnalyzer::dealParamDec(NodeType& paramDec, FunDef& funDef) {
        auto& specifier = paramDec->subNodes[0];
        auto& varDec = paramDec->subNodes[1];
        if(hadDeclareInCurrentScope(std::get<string>(varDec->value))) {
            appendError(3, specifier->loc, std::get<string>(varDec->value) + SEMANTIC_ERROR_TEMPLATE[2]);
            return false;
        }
        splDebugLog("Parameter Add, Name:  " + std::get<string>(varDec->value) + "\tType: " + std::get<string>(specifier->value));
        if(isInt(specifier) || isFloat(specifier) || isChar(specifier)) {  // primary type & array
            if(varDec->subNodes.size() == 1) {                             //  primary type
                funDef.argTypes.push_back(specifier->valueType);
            } else {  //  array
                AryDef aryDef{ specifier->valueType };
                if(!dealArray(aryDef, varDec)) {
                    return false;
                }
                funDef.argTypes.push_back(ValueType::ARRAY);
                funDef.argAry.emplace_back(funDef.argTypes.size() - 1, aryDef);
            }
        } else {  // struct
            if(!hadDefined(std::get<string>(specifier->value), ValueType::STRUCT)) {
                appendError(16, specifier->loc, SEMANTIC_ERROR_TEMPLATE[15]);
                return false;
            }
            funDef.argTypes.push_back(ValueType::STRUCT);
        }
        funDef.argIds.push_back(std::get<string>(varDec->value));
        return true;
    }

    bool SemanticAnalyzer::dealVarDec(const NodeType& specifier, NodeType& varDec) {
        if(hadDeclareInCurrentScope(std::get<string>(varDec->value))) {
            appendError(3, specifier->loc, std::get<string>(varDec->value) + SEMANTIC_ERROR_TEMPLATE[2]);
            return false;
        }
        if(isInt(specifier) || isFloat(specifier) || isChar(specifier)) {  // primary type & array
            if(varDec->subNodes.size() == 1) {                             //  primary type
                if(!declareVariable(specifier, varDec)) {
                    return false;
                }
            } else {  //  array
                AryDef aryDef{ specifier->valueType };
                if(!dealArray(aryDef, varDec)) {
                    return false;
                }
                if(!declareVariable(specifier, varDec)) {
                    return false;
                }
            }
        } else {  // struct
            if(!hadDefined(std::get<string>(specifier->value), ValueType::STRUCT)) {
                appendError(16, specifier->loc, SEMANTIC_ERROR_TEMPLATE[15]);
                return false;
            }
            if(!declareVariable(specifier, varDec)) {
                return false;
            }
        }
        return true;
    }

    void SemanticAnalyzer::dealExtDef(NodeType& extDef) {
        if(extDef->subNodes.size() == 2) {  // extDef -> specifier SEMI
            return;
        } else if(static_cast<TOKEN_TYPE>(extDef->subNodes[2]->type) ==
                  TOKEN_TYPE::SEMI) {  // extDef -> specifier extDecList SEMI, deal with iteration
            auto& specifier = extDef->subNodes[0];
            auto* idNode = &extDef->subNodes[1];
            while(true) {
                auto& subNodes = (*idNode)->subNodes;
                declareVariable(specifier, subNodes.front());
                if(subNodes.size() == 1) {
                    break;
                }
                idNode = &subNodes.back();
            }
        } else {  // extDef -> specifier FunDec CompSt
            if(auto funDef = dealFunDec(extDef->subNodes[0], extDef->subNodes[1])) {
                if(!dealCompSt(funDef.value(), extDef->subNodes[2])) {
                    getSymbolTable().erase(
                        std::get<string>(extDef->subNodes[0]->value));  // remove error function from symbol table
                }
            }
        }
    }

    bool SemanticAnalyzer::dealDec(const NodeType& specifier, NodeType& dec) {
        if(!dealVarDec(specifier, dec->subNodes[0])) {
            return false;
        }
        if(dec->subNodes.size() == 1) {  // dec -> varDec
            return true;
        }
        // dec -> varDec ASSIGNOP exp

        //  TODO
        return true;
        // return
    }

    ValueType SemanticAnalyzer::dealExp(NodeType& exp) {
        // TODO
    }

    bool dealArgs(NodeType& args) {
        // TODO
    }

    void SemanticAnalyzer::dealSpecifier(NodeType& specifier) {
        auto& subNode = specifier->subNodes[0];
        if(subNode->valueType == ValueType::TYPE) {
            return;
        }
        dealStructSpecifier(subNode);
    }

    void SemanticAnalyzer::dealStructSpecifier(NodeType& structSpecifier) {
        if(structSpecifier->subNodes.size() == 2) {
            return;
        }
        dealDefList(structSpecifier->subNodes[3]);  // TODO
    }

    bool SemanticAnalyzer::dealDecList(NodeType& specifier, NodeType& decList) {
        auto* decListNode = &decList;
        while(true) {
            if(!dealDec(specifier, (*decListNode)->subNodes[0])) {
                return false;
            }
            if((*decListNode)->subNodes.size() == 1) {
                break;
            }
            decListNode = &(*decListNode)->subNodes[2];
        }
        return true;
    }

    bool SemanticAnalyzer::dealDef(NodeType& def) {  // iterate the declist
        return dealDecList(def->subNodes[0], def->subNodes[1]);
    }

    bool SemanticAnalyzer::dealDefList(NodeType& defList) {
        auto* defListNode = &defList;
        while(true) {
            if(static_cast<TOKEN_TYPE>((*defListNode)->type) == TOKEN_TYPE::NOTHING) {
                break;
            }
            if(!dealDef((*defListNode)->subNodes[0])) {
                return false;
            }
            defListNode = &(*defListNode)->subNodes[1];
        }
        return true;
    }

    bool SemanticAnalyzer::dealDefList(const std::string& structId, NodeType& defList) {
        return true;
        // TODO
    }

    bool SemanticAnalyzer::declareVariable(const NodeType& specifier, const NodeType& var) {
        auto& varTable = m_varTables.back();
        if(hadDeclareInCurrentScope(std::get<string>(var->value))) {
            appendError(3, specifier->loc, std::get<string>(var->value) + SEMANTIC_ERROR_TEMPLATE[2]);
            return false;
        }
        splDebugLog("Variable Add, Name:  " + std::get<string>(var->value) + "\tType: " + std::get<string>(specifier->value));
        switch(specifier->valueType) {
            case ValueType::TYPE:
                varTable[std::get<string>(var->value)] = { var->valueType, std::get<string>(var->typeValue), 1 };
                break;
            case ValueType::STRUCT:
                varTable[std::get<string>(var->value)] = { ValueType::STRUCT, std::get<string>(var->typeValue), 1 };
                break;
            default:
                break;
        }
        return true;
    }

    void SemanticAnalyzer::dealExtDefList(NodeType& extDefList) {
        while(true) {
            switch(static_cast<TOKEN_TYPE>(extDefList->type)) {
                case TOKEN_TYPE::NON_TERMINAL: {
                    dealExtDef(extDefList->subNodes[0]);
                    dealExtDefList(extDefList->subNodes[1]);
                }
                case TOKEN_TYPE::NOTHING:
                    return;
                default:
                    splDebugLog("unknown type in dealExtdefList");
                    return;
            }
        }
    }

    std::optional<FunDef> SemanticAnalyzer::dealFunDec(NodeType& specifier, NodeType& funDec) {
        if(hadDefined(std::get<string>(funDec->value), ValueType::FUNCTION)) {
            appendError(4, specifier->loc, "Function " + std::get<string>(funDec->value) + SEMANTIC_ERROR_TEMPLATE[3]);
            return std::nullopt;
        }
        FunDef funDef;
        if(specifier->valueType == ValueType::TYPE) {
            funDef.returnType = string2ValueType(specifier->value);
            funDef.returnTypeValue = std::get<string>(specifier->value);
        } else {
            if(!hadDefined(std::get<string>(specifier->value), ValueType::STRUCT)) {
                appendError(16, specifier->loc, SEMANTIC_ERROR_TEMPLATE[15]);
                return std::nullopt;
            }
            funDef.returnType = ValueType::STRUCT;
            funDef.returnTypeValue = std::get<string>(specifier->typeValue);
        }
        if(funDec->subNodes.size() == 3) {
            goto AddDefine;
        } else if(!dealVarList(funDec->subNodes[2], funDef)) {
            return std::nullopt;
        }
    AddDefine:
        getSymbolTable().insert({ std::get<string>(funDec->value), { ValueType::FUNCTION, funDef } });
        return funDef;
    }

    bool SemanticAnalyzer::dealCompSt(const FunDef& funDef, NodeType& compSt) {
        if(!(dealDefList(compSt->subNodes[1]) && dealStmtList(funDef, compSt->subNodes[2]))) {
            return false;
        }
        return true;
    }

    bool SemanticAnalyzer::dealStmtList(const FunDef& funDef, NodeType& stmtList) {
        // TODO
        return true;
    }

    bool SemanticAnalyzer::analyze() {
        m_varTables.clear();
        m_symbolTables.clear();
        waitToken.clear();
        m_varTables.emplace_back();
        m_symbolTables.emplace_back();
        for(auto& rootNode : frontage.m_parseTree) {
            for(auto& extDefListNode : rootNode->subNodes) {
                dealExtDefList(extDefListNode);
            }
        }
        return m_errors.empty();
    }

    string SemanticAnalyzer::getErrors() const {
        std::stringstream ss;
        for(const auto& error : m_errors) {
            ss << error << '\n';
        }
        return ss.str();
    }

    SymbolTableble& SemanticAnalyzer::getSymbolTable() {
        return m_symbolTables.back();
    }

    bool SemanticAnalyzer::hadDefined(const std::string& id, ValueType type) {
        auto& symbolTable = getSymbolTable();
        return symbolTable.count(id) != 0 && symbolTable[id].type == type;
    }

    bool SemanticAnalyzer::hadDeclareInCurrentScope(const std::string& id) {
        return m_varTables.back().count(id) != 0;
    }

    bool SemanticAnalyzer::hadDeclareInAllScope(const std::string& id) {
        bool find = false;
        for_each(m_varTables.rbegin(), m_varTables.rend(), [&](const auto& varTable) {
            if(varTable.count(id) != 0) {
                find = true;
                return;
            }
        });
        return find;
    }
}  // namespace spl
