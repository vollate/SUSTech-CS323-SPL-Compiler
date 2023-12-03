#include "Semantic.hpp"
#include "Frontage.hpp"
#include "Parser.hpp"

#include <algorithm>
#include <any>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>

namespace spl {
    using std::string;
    namespace {
        bool isEqual(const AryDef &lhs, size_t lhsLevel, const AryDef &rhs, size_t rhsLevel) {
            if (lhs.type != rhs.type) {
                return false;
            }
            auto lhsIter = lhs.subAryLength.begin() + lhsLevel;
            auto rhsIter = rhs.subAryLength.begin() + rhsLevel;
            while (lhsIter != lhs.subAryLength.end()) {
                if (*lhsIter != *rhsIter) {
                    return false;
                }
                ++lhsIter;
                ++rhsIter;
            }
            return true;
        }

        bool isEqual(const StructDef &lhs, const StructDef &rhs) {
            if (lhs.memberTypes.size() != rhs.memberTypes.size()) {
                return false;
            }
            auto lhsIter = lhs.memberTypes.begin();
            auto rhsIter = rhs.memberTypes.begin();
            size_t ary_index = 0;
            while (lhsIter != lhs.memberTypes.end()) {
                if (*lhsIter != *rhsIter) {
                    return false;
                } else if (*lhsIter == ValueType::ARRAY) {
                }
                ++lhsIter;
                ++rhsIter;
            }
            return true;
        }

        ValueType string2ValueType(const variant_type &var) {
            auto str = std::get<string>(var);
            if (str == "int") {
                return ValueType::INT;
            } else if (str == "float") {
                return ValueType::FLOAT;
            } else if (str == "char") {
                return ValueType::CHAR;
            }
            return ValueType::NONE;
        }

        void updateValueType(NodeType &node) {
            node->valueType = string2ValueType(node->value);
        }

        bool isInt(const NodeType &node) {
            return node->valueType == ValueType::INT;
        }

        bool isFloat(const NodeType &node) {
            return node->valueType == ValueType::FLOAT;
        }

        bool isChar(const NodeType &node) {
            return node->valueType == ValueType::CHAR;
        }

        bool canCompare(const NodeType &lhs, const NodeType &rhs) {
            return (isInt(lhs) || isFloat(lhs) || isChar(lhs)) && (isInt(rhs) || isFloat(rhs) || isChar(rhs));
        }

        bool deReferenceArray(AryDef &aryDef, NodeType &exp) {
            auto *expNode = &exp;
            while (true) {
                if ((*expNode)->subNodes.size() == 1) {
                    break;
                }
                int length = std::get<int>((*expNode)->subNodes[2]->value);
                if (length < 0) {
                    return false;
                }
                aryDef.subAryLength.push_front(length);
                expNode = &(*expNode)->subNodes[0];
            }
            return true;
        }
    }  // namespace

    void SemanticAnalyzer::appendError(int errorId, const location &location, const std::string &msg) {
        std::stringstream ss;
        ss << "Error type " << errorId << " at Line " << location.end.line << ": " << msg;
#ifdef SPL_DEBUG
        cerr << ss.str() << '\n';
#endif
        m_errors.push_back(ss.str());
    }

    std::optional<ValueType> SemanticAnalyzer::processArrayExp(NodeType &exp) {
        processExp(exp->subNodes[0]);
        processExp(exp->subNodes[2]);
        if (exp->subNodes[0]->valueType != ValueType::ARRAY) {
            appendError(10, exp->loc, SEMANTIC_ERROR_TEMPLATE[9]);
            return std::nullopt;
        } else if (exp->subNodes[2]->valueType != ValueType::INT) {
            appendError(12, exp->loc, SEMANTIC_ERROR_TEMPLATE[11]);
            return std::nullopt;
        }
        exp->aryLevel = ++exp->subNodes[0]->aryLevel;
        auto var = getVarNode(std::get<string>(exp->subNodes[0]->value));
        AryDef aryDef = std::any_cast<AryDef>(var->val);
        if (exp->subNodes[0]->aryLevel == aryDef.subAryLength.size())
            return aryDef.type;
        return ValueType::ARRAY;
    }

    bool SemanticAnalyzer::processArray(AryDef &aryDef, NodeType &varDec) {
        auto *varDecNode = &varDec;
        while (true) {
            if ((*varDecNode)->subNodes.size() == 1) {
                break;
            }
            int length = std::get<int>((*varDecNode)->subNodes[2]->value);
            if (length < 0) {
                return false;
            }
            aryDef.subAryLength.push_front(length);
            varDecNode = &(*varDecNode)->subNodes[0];
        }
        return true;
    }

    bool SemanticAnalyzer::processArgs(FunDef &funDef, NodeType &args) {
        auto *argsNode = &args;
        size_t argsCnt = 0;
        while (true) {
            if (argsCnt >= funDef.argTypes.size()) {
                appendError(9, (*argsNode)->loc,
                            SEMANTIC_ERROR_TEMPLATE[8] + std::to_string(funDef.argIds.size()) + " but got " +
                            std::to_string(argsCnt + 1));
                return false;
            }
            processExp((*argsNode)->subNodes[0]);
            if ((*argsNode)->subNodes[0]->valueType != funDef.argTypes[argsCnt]) {
                appendError(19, (*argsNode)->loc, SEMANTIC_ERROR_TEMPLATE[18]);
                return false;
            }
            ++argsCnt;
            if ((*argsNode)->subNodes.size() == 1) {
                break;
            }
            argsNode = &(*argsNode)->subNodes[2];
        }
        if (argsCnt != funDef.argTypes.size()) {
            appendError(9, (*argsNode)->loc,
                        SEMANTIC_ERROR_TEMPLATE[8] + std::to_string(funDef.argIds.size()) + " but got " +
                        std::to_string(argsCnt));
            return false;
        }
        return true;
    }

    bool SemanticAnalyzer::canAssign(const NodeType &lhs, const NodeType &rhs) {
        switch (lhs->valueType) {
            case ValueType::NONE:
                //                appendError(5, lhs->loc, SEMANTIC_ERROR_TEMPLATE[4]);
                return false;
            default:
                break;
        }
        if (lhs->isRvalue) {
            appendError(6, lhs->loc, SEMANTIC_ERROR_TEMPLATE[5]);
            return false;
        }
        if (lhs->valueType == rhs->valueType) {
            if (lhs->valueType == ValueType::ARRAY) {
                auto lhsAryDef = getVarNode(std::get<string>(lhs->value));
                auto rhsAryDef = getVarNode(std::get<string>(rhs->value));
                if (isEqual(std::any_cast<AryDef>(lhsAryDef->val), lhs->aryLevel, std::any_cast<AryDef>(rhsAryDef->val),
                            rhs->aryLevel)) {
                    return true;
                } else {
                    return false;
                }
            }
            if (lhs->valueType == ValueType::STRUCT) {  // struct can't be undefined since it's checked
                auto lhsVar = getVarNode(std::get<string>(lhs->value));
                auto rhsVar = getVarNode(std::get<string>(rhs->value));
                return isEqual(std::any_cast<StructDef>(lhsVar->val), std::any_cast<StructDef>(rhsVar->val));
            }
            return true;
        }
        ValueType lhsValueType = (lhs->valueType == ValueType::ID ? string2ValueType(lhs->value) : lhs->valueType);
        ValueType rhsValueType = (rhs->valueType == ValueType::ID ? string2ValueType(rhs->value) : rhs->valueType);
        switch (lhsValueType) {
            case ValueType::FLOAT:
                switch (rhsValueType) {
                    case ValueType::FLOAT:
                    case ValueType::INT:
                    case ValueType::CHAR:
                        return true;
                    default:
                        break;
                }
                break;
            case ValueType::INT:
                switch (rhsValueType) {
                    case ValueType::INT:
                    case ValueType::CHAR:
                        return true;
                    default:
                        break;
                }
                break;
            case ValueType::CHAR:
                if (rhsValueType == ValueType::CHAR) {
                    return true;
                }
                break;
            default:
                break;
        }
        return false;
    }

    bool SemanticAnalyzer::processVarList(NodeType &varList, FunDef &funDef) {
        auto *varNode = &varList;
        m_varTables.emplace_back();
        while (true) {
            if (!processParamDec((*varNode)->subNodes[0], funDef)) {
                m_varTables.pop_back();
                return false;
            } else if ((*varNode)->subNodes.size() == 1) {
                break;
            }
            varNode = &(*varNode)->subNodes[2];
        }
        return true;
    }

    bool SemanticAnalyzer::processParamDec(NodeType &paramDec, FunDef &funDef) {
        auto &specifier = paramDec->subNodes[0];
        auto &varDec = paramDec->subNodes[1];
        processSpecifier(specifier);
        if (hadDeclareInCurrentScope(std::get<string>(varDec->value))) {
            appendError(3, specifier->loc, std::get<string>(varDec->value) + SEMANTIC_ERROR_TEMPLATE[2]);
            return false;
        }
        updateValueType(specifier);
        splDebugLog("Parameter Add, Name:  " + std::get<string>(varDec->value) + "\tType: " +
                    std::get<string>(specifier->value));
        if (isInt(specifier) || isFloat(specifier) || isChar(specifier)) {  // primary type & array
            if (varDec->subNodes.size() == 1) {                             //  primary type
                funDef.argTypes.push_back(specifier->valueType);
                declareVariable(specifier, varDec);
            } else {  //  array
                AryDef aryDef{specifier->valueType};
                if (!processArray(aryDef, varDec)) {
                    return false;
                }
                funDef.argTypes.push_back(ValueType::ARRAY);
                funDef.argAry.emplace_back(funDef.argTypes.size() - 1, aryDef);
                declareVariable(specifier, varDec);
            }
        } else {  // struct
            if (!hadDefined(std::get<string>(specifier->value), ValueType::STRUCT)) {
                appendError(16, specifier->loc, SEMANTIC_ERROR_TEMPLATE[15]);
                return false;
            }
            funDef.argTypes.push_back(ValueType::STRUCT);
            declareVariable(specifier, varDec);
        }
        funDef.argIds.push_back(std::get<string>(varDec->value));
        return true;
    }

    bool SemanticAnalyzer::processVarDec(NodeType &specifier, NodeType &varDec, bool structDef) {
        if (hadDeclareInCurrentScope(std::get<string>(varDec->value))) {
            appendError(3, specifier->loc, std::get<string>(varDec->value) + SEMANTIC_ERROR_TEMPLATE[2]);
            return false;
        }
        auto storeSpec = specifier->value;
        updateValueType(specifier);
        if (isInt(specifier) || isFloat(specifier) || isChar(specifier)) {  // primary type & array
            if (varDec->subNodes.size() == 1) {                             //  primary type
                if (!declareVariable(specifier, varDec)) {
                    if (!structDef)
                        varDec->value = storeSpec;
                    return false;
                }
                if (!structDef)
                    varDec->value = storeSpec;
            } else {  //  array
                AryDef aryDef{specifier->valueType};
                if (!processArray(aryDef, varDec)) {
                    return false;
                }
                m_varTables.back()[std::get<string>(varDec->value)] = {ValueType::ARRAY,
                                                                       std::get<string>(varDec->typeValue),
                                                                       aryDef};
            }
        } else {  // struct, useless since no struct before id in the induction
            //            return false;
            if (!structDef)
                varDec->value = storeSpec;
            if (!hadDefined(std::get<string>(specifier->value), ValueType::STRUCT)) {
                appendError(16, specifier->loc, SEMANTIC_ERROR_TEMPLATE[15]);
                return false;
            }
            specifier->valueType = ValueType::STRUCT;
            if (!declareVariable(specifier, varDec)) {
                return false;
            }
        }
        return true;
    }

    void SemanticAnalyzer::processExtDef(NodeType &extDef) {
        processSpecifier(extDef->subNodes[0]);
        if (extDef->subNodes.size() == 2) {  // extDef -> specifier SEMI
            return;
        } else if (static_cast<TOKEN_TYPE>(extDef->subNodes[2]->type) ==
                   TOKEN_TYPE::SEMI) {  // extDef -> specifier extDecList SEMI, process with iteration
            auto &specifier = extDef->subNodes[0];
            auto *idNode = &extDef->subNodes[1];
            while (true) {
                auto &subNodes = (*idNode)->subNodes;
                declareVariable(specifier, subNodes.front());
                if (subNodes.size() == 1) {
                    break;
                }
                idNode = &subNodes.back();
            }
        } else {  // extDef -> specifier FunDec CompSt
            if (auto funDef = processFunDec(extDef->subNodes[0], extDef->subNodes[1])) {
                if (!processCompSt(funDef.value(), extDef->subNodes[2])) {
                    splDebugLog("Function " + std::get<string>(extDef->subNodes[1]->value) + " has error, remove it");
                    getSymbolTable().erase(
                            std::get<string>(extDef->subNodes[0]->value));  // remove error function from symbol table
                }
            }
            m_varTables.pop_back();
        }
    }

    bool SemanticAnalyzer::processDec(StructDef &structDef, NodeType &specifier, NodeType &dec) {  // for struct def
        if (!processVarDec(specifier, dec->subNodes[0], true)) {
            return false;
        }
        structDef.memberTypes.push_back(specifier->valueType);
        switch (specifier->valueType) {
            case ValueType::ARRAY:
                structDef.memberAry.push_back({structDef.memberTypes.size() - 1,
                                               std::any_cast<AryDef>(
                                                       getVarNode(std::get<string>(specifier->value))->val)});
                break;
            case ValueType::STRUCT:
                structDef.memberStruct.push_back(
                        {structDef.memberTypes.size() - 1, std::get<string>(specifier->value)});
            default:
                break;
        }
        structDef.memberIds.push_back(std::get<string>(dec->subNodes[0]->value));
        if (dec->subNodes.size() == 1) {  // dec -> varDec
            return true;
        }
        // dec -> varDec ASSIGN exp
        processExp(dec->subNodes[2]);
        if (!canAssign(dec->subNodes[0], dec->subNodes[2])) {
            appendError(5, dec->loc, SEMANTIC_ERROR_TEMPLATE[4]);
            return false;
        }
        return true;
    }

    bool SemanticAnalyzer::processDec(NodeType &specifier, NodeType &dec) {
        if (!processVarDec(specifier, dec->subNodes[0], specifier->valueType == ValueType::STRUCT)) {
            return false;
        }
        if (dec->subNodes.size() == 1) {  // dec -> varDec
            return true;
        }
        // dec -> varDec ASSIGN exp
        processExp(dec->subNodes[2]);
        if (!canAssign(dec->subNodes[0], dec->subNodes[2])) {
            appendError(5, dec->loc, SEMANTIC_ERROR_TEMPLATE[4]);
            return false;
        }
        return true;
    }

    void SemanticAnalyzer::processExp(NodeType &exp) {
        auto &subNodes = exp->subNodes;
        if (subNodes.size() == 1) {
            if (static_cast<TOKEN_TYPE>(subNodes[0]->type) == TOKEN_TYPE::ID) {
                if (auto varNode = getVarNode(std::get<string>(subNodes[0]->value))) {
                    exp->valueType = varNode->type;
                } else if (exp->isSubStruct && exp->valueType == ValueType::STRUCT) {}
                else {
                    appendError(1, exp->loc, std::get<string>(exp->subNodes[0]->value) + SEMANTIC_ERROR_TEMPLATE[0]);
                }
            } else {
                switch (static_cast<TOKEN_TYPE>(subNodes[0]->type)) {
                    case TOKEN_TYPE::INT:
                    case TOKEN_TYPE::FLOAT:
                    case TOKEN_TYPE::CHAR:
                        exp->isRvalue = true;
                    default:
                        break;
                }
            }
        } else if (subNodes.size() == 2) {  //+/- exp
            processExp(subNodes[1]);
            if (static_cast<TOKEN_TYPE>(subNodes[0]->type) == TOKEN_TYPE::NOT &&
                (subNodes[1]->valueType != ValueType::INT)) {
                appendError(7, exp->loc, SEMANTIC_ERROR_TEMPLATE[6]);
            }
            if (!(isInt(subNodes[1]) || !isFloat(subNodes[1]))) {
                appendError(7, exp->loc, SEMANTIC_ERROR_TEMPLATE[6]);
            }
            exp->valueType = subNodes[1]->valueType;
        } else if (subNodes.size() == 3) {
            if (static_cast<TOKEN_TYPE>(subNodes[0]->type) == TOKEN_TYPE::LP) {
                processExp(subNodes[2]);
                exp->valueType = subNodes[2]->valueType;
                exp->value = subNodes[2]->value;
                return;
            } else if (static_cast<TOKEN_TYPE>(subNodes[1]->type) == TOKEN_TYPE::LP) {  // process function call
                if (auto defNode = getDefNode(std::get<string>(subNodes[0]->value), ValueType::FUNCTION)) {
                    auto funDef = std::get<FunDef>(defNode->val);
                    if (!funDef.argIds.empty()) {
                        appendError(9, exp->loc,
                                    SEMANTIC_ERROR_TEMPLATE[8] + std::to_string(funDef.argIds.size()) + " but got 0");
                    } else {
                        exp->valueType = funDef.returnType;
                    }
                    return;
                } else {
                    if (hadDeclareInAllScope(std::get<string>(subNodes[0]->value))) {
                        appendError(11, exp->loc, SEMANTIC_ERROR_TEMPLATE[10]);
                    } else {
                        appendError(2, exp->loc, std::get<string>(subNodes[0]->value) + SEMANTIC_ERROR_TEMPLATE[1]);
                    }
                }
            } else if (static_cast<TOKEN_TYPE>(subNodes[1]->type) == TOKEN_TYPE::DOT) {  // process struct member access
                if (static_cast<TOKEN_TYPE>(subNodes[2]->type) != TOKEN_TYPE::ID) {
                    appendError(14, exp->loc, SEMANTIC_ERROR_TEMPLATE[13]);
                } else {
                    processExp(subNodes[0]);
                    if (subNodes[0]->valueType == ValueType::STRUCT) {
                        auto varNode = getVarNode(std::get<string>(subNodes[0]->value));
                        if (!varNode && !subNodes[0]->isSubStruct) {
                            appendError(1, exp->loc, std::get<string>(subNodes[0]->value) + SEMANTIC_ERROR_TEMPLATE[0]);
                            return;
                        } else if (varNode->type != ValueType::STRUCT) {
                            appendError(13, exp->loc, SEMANTIC_ERROR_TEMPLATE[12]);
                            return;
                        }
                        if (!subNodes[0]->isSubStruct &&
                            !hadDefined(std::any_cast<string>(varNode->val), ValueType::STRUCT)) {
                            appendError(13, exp->loc, SEMANTIC_ERROR_TEMPLATE[12]);
                        } else {
                            auto structDef = subNodes[0]->isSubStruct ?
                                             m_structSubStruct :
                                             std::get<StructDef>(getDefNode(std::any_cast<string>(varNode->val),
                                                                            ValueType::STRUCT)->val);
                            for (int i = 0; i < structDef.memberIds.size(); ++i) {
                                if (structDef.memberIds[i] == std::get<string>(subNodes[2]->value)) {
                                    switch (structDef.memberTypes[i]) {
                                        case ValueType::STRUCT: {
                                            exp->value = structDef.memberIds[i];
                                            exp->isSubArray = false;
                                            exp->isSubStruct = true;
                                            auto res = std::find_if(structDef.memberStruct.begin(),
                                                                    structDef.memberStruct.end(),
                                                                    [i](auto &v) { return i == v.first; });
                                            if (auto def = getDefNode(res->second, ValueType::STRUCT)) {
                                                m_structSubStruct = std::get<StructDef>(def->val);
                                            } else {
                                                // appendError()
                                            }
                                            break;
                                        }
                                        case ValueType::ARRAY: {
                                            exp->isSubArray = true;
                                            exp->isSubStruct = false;
                                            auto res = std::find_if(structDef.memberAry.begin(),
                                                                    structDef.memberAry.end(),
                                                                    [i](auto &v) { return i == v.first; });
                                            m_structSubAry = res->second;
                                            break;
                                        }
                                        default:
                                            exp->isSubArray = exp->isSubStruct = false;
                                            break;
                                    }
                                    exp->valueType = structDef.memberTypes[i];
                                    return;
                                }
                            }
                            appendError(14, exp->loc, SEMANTIC_ERROR_TEMPLATE[13]);
                        }
                    } else {
                        appendError(13, exp->loc, SEMANTIC_ERROR_TEMPLATE[12]);
                    }
                }
            }
            processExp(subNodes[0]);
            processExp(subNodes[2]);
            switch (static_cast<TOKEN_TYPE>(subNodes[1]->type)) {
                case TOKEN_TYPE::ASSIGN: {
                    if (!canAssign(subNodes[0], subNodes[2])) {
                        appendError(5, exp->loc, SEMANTIC_ERROR_TEMPLATE[4]);
                    } else {
                        exp->valueType = subNodes[0]->valueType;
                    }
                    break;
                }
                    // logic
                case TOKEN_TYPE::AND:
                case TOKEN_TYPE::OR:
                    if (!isInt(subNodes[0]) || !isInt(subNodes[2])) {
                        appendError(17, exp->loc, SEMANTIC_ERROR_TEMPLATE[16]);
                    } else {
                        exp->valueType = ValueType::INT;
                    }
                    break;
                    // compare
                case TOKEN_TYPE::LT:
                case TOKEN_TYPE::LE:
                case TOKEN_TYPE::GT:
                case TOKEN_TYPE::GE:
                case TOKEN_TYPE::NE:
                case TOKEN_TYPE::EQ:
                    if (!canCompare(subNodes[0], subNodes[2])) {
                        appendError(18, exp->loc, SEMANTIC_ERROR_TEMPLATE[17]);
                    } else {
                        exp->valueType = ValueType::INT;
                    }
                    break;
                    // arthimetic
                case TOKEN_TYPE::PLUS:
                case TOKEN_TYPE::MINUS:
                case TOKEN_TYPE::MUL:
                case TOKEN_TYPE::DIV:
                    if (subNodes[0]->valueType != subNodes[2]->valueType) {
                        appendError(7, exp->loc, SEMANTIC_ERROR_TEMPLATE[6]);
                        exp->valueType = ValueType::NONE;
                    } else {
                        switch (subNodes[0]->valueType) {
                            case ValueType::ARRAY:
                            case ValueType::STRUCT:
                            case ValueType::NONE:
                                exp->valueType = ValueType::NONE;
                                appendError(7, exp->loc, SEMANTIC_ERROR_TEMPLATE[6]);
                                break;
                            default:
                                exp->valueType = subNodes[0]->valueType;
                        }
                    }
                    break;
                default:
                    break;
            }
        } else if (subNodes.size() == 4) {
            if (static_cast<TOKEN_TYPE>(subNodes[1]->type) == TOKEN_TYPE::LB) {  // Array Check
                if (auto type = processArrayExp(exp)) {
                    exp->valueType = type.value();
                } else {
                    exp->valueType = ValueType::NONE;
                }
            } else {  // Function Call
                if (auto funDef = getDefNode(std::get<string>(subNodes[0]->value), ValueType::FUNCTION)) {
                    if (!processArgs(std::get<FunDef>(funDef->val), subNodes[2])) {
                        exp->valueType = ValueType::NONE;
                    } else {
                        exp->valueType = std::get<FunDef>(funDef->val).returnType;
                    }
                } else {
                    if (hadDeclareInAllScope(std::get<string>(subNodes[0]->value))) {
                        appendError(11, exp->loc, SEMANTIC_ERROR_TEMPLATE[10]);
                    } else {
                        appendError(2, exp->loc, std::get<string>(subNodes[0]->value) + SEMANTIC_ERROR_TEMPLATE[1]);
                    }
                }
            }
        }
    }

    bool SemanticAnalyzer::processSpecifier(NodeType &specifier) {
        if (specifier->valueType != ValueType::STRUCT) {
            updateValueType(specifier);
            return true;
        }
        return processStructSpecifier(specifier->subNodes[0]);
    }

    bool SemanticAnalyzer::processStructSpecifier(NodeType &structSpecifier) {
        if (hadDefined(std::get<string>(structSpecifier->value), ValueType::STRUCT)) {
            if (structSpecifier->subNodes.size() != 2) {
                appendError(15, structSpecifier->loc, SEMANTIC_ERROR_TEMPLATE[14]);
                return false;
            } else {
                structSpecifier->valueType = ValueType::STRUCT;
            }
        } else {
            if (structSpecifier->subNodes.size() == 2) {
                appendError(16, structSpecifier->loc, SEMANTIC_ERROR_TEMPLATE[15]);
                return false;
            }
            StructDef structDef;
            if (!processDefList(structDef, structSpecifier->subNodes[3])) {
                return false;
            }
            insertSymbolTable(std::get<string>(structSpecifier->value), {ValueType::STRUCT, structDef});
        }
        return true;
    }

    bool SemanticAnalyzer::processDecList(NodeType &specifier, NodeType &decList) {
        auto *decListNode = &decList;
        bool value = true;
        while (true) {
            if (!processDec(specifier, (*decListNode)->subNodes[0])) {
                // return false;
                value = false;
            }
            if ((*decListNode)->subNodes.size() == 1) {
                break;
            }
            decListNode = &(*decListNode)->subNodes[2];
        }
        return value;
    }

    bool SemanticAnalyzer::processDecList(StructDef &structDef, NodeType &specifier, NodeType &decList) {
        auto *decListNode = &decList;
        bool value = true;
        while (true) {
            if (!processDec(structDef, specifier, (*decListNode)->subNodes[0])) {
                //                return false;
                value = false;
            }
            if ((*decListNode)->subNodes.size() == 1) {
                break;
            }
            decListNode = &(*decListNode)->subNodes[2];
        }
        return value;
    }

    bool SemanticAnalyzer::processDef(NodeType &def) {  // iterate the declist
        bool value = processSpecifier(def->subNodes[0]);
        return processDecList(def->subNodes[0], def->subNodes[1]) && value;
    }

    bool
    SemanticAnalyzer::processDef(StructDef &stuctDef, NodeType &def) {  // iterate the DecLit, for struct definition
        bool value = processSpecifier(def->subNodes[0]);
        return processDecList(stuctDef, def->subNodes[0], def->subNodes[1]) && value;
    }

    bool SemanticAnalyzer::processDefList(NodeType &defList) {
        auto *defListNode = &defList;
        bool value = true;
        while (true) {
            if (static_cast<TOKEN_TYPE>((*defListNode)->type) == TOKEN_TYPE::NOTHING) {
                break;
            }
            if (!processDef((*defListNode)->subNodes[0])) {
                // return false;
                value = false;  // force analyze all
            }
            defListNode = &(*defListNode)->subNodes[1];
        }
        return value;
    }

    bool SemanticAnalyzer::processDefList(StructDef &structDef, NodeType &defList) {
        m_varTables.emplace_back();
        auto *defListNode = &defList;
        bool value = true;
        while (true) {
            if (static_cast<TOKEN_TYPE>((*defListNode)->type) == TOKEN_TYPE::NOTHING) {
                break;
            }
            if (!processDef(structDef, (*defListNode)->subNodes[0])) {
                // return false;
                value = false;
            }
            if ((*defListNode)->subNodes.size() == 1) {
                break;
            }
            defListNode = &(*defListNode)->subNodes[1];
        }
        m_varTables.pop_back();
        return value;
    }

    bool SemanticAnalyzer::declareVariable(NodeType &specifier, const NodeType &var) {
        auto &varTable = m_varTables.back();
        if (hadDeclareInCurrentScope(std::get<string>(var->value))) {
            appendError(3, specifier->loc, std::get<string>(var->value) + SEMANTIC_ERROR_TEMPLATE[2]);
            return false;
        }
        splDebugLog("Variable Add, Name:  " + std::get<string>(var->value) + "\tType: " +
                    std::get<string>(specifier->value));
        switch (specifier->valueType) {
            case ValueType::INT:
            case ValueType::FLOAT:
            case ValueType::CHAR:
                varTable[std::get<string>(var->value)] = {specifier->valueType, std::get<string>(var->typeValue), 1};
                break;
            case ValueType::STRUCT:
                varTable[std::get<string>(var->value)] = {specifier->valueType, std::get<string>(var->typeValue),
                                                          std::get<string>(specifier->value)};
                break;
            default:
                break;
        }
        return true;
    }

    void SemanticAnalyzer::processExtDefList(NodeType &extDefList) {
        while (true) {
            switch (static_cast<TOKEN_TYPE>(extDefList->type)) {
                case TOKEN_TYPE::NON_TERMINAL: {
                    processExtDef(extDefList->subNodes[0]);
                    processExtDefList(extDefList->subNodes[1]);
                }
                case TOKEN_TYPE::NOTHING:
                    return;
                default:
                    splDebugLog("unknown type in processExtdefList");
                    return;
            }
        }
    }

    std::optional<FunDef> SemanticAnalyzer::processFunDec(NodeType &specifier, NodeType &funDec) {
        bool returnNullOpt = false;
        if (hadDefined(std::get<string>(funDec->value), ValueType::FUNCTION)) {
            appendError(4, specifier->loc, "Function " + std::get<string>(funDec->value) + SEMANTIC_ERROR_TEMPLATE[3]);
            return std::nullopt;
        }
        FunDef funDef;
        if (specifier->valueType != ValueType::STRUCT) {
            funDef.returnType = string2ValueType(specifier->value);
            funDef.returnTypeName = std::get<string>(specifier->value);
        } else {
            if (!hadDefined(std::get<string>(specifier->value), ValueType::STRUCT)) {
                appendError(16, specifier->loc, SEMANTIC_ERROR_TEMPLATE[15]);
                returnNullOpt = true;
            }
            funDef.returnType = ValueType::STRUCT;
            funDef.returnTypeName = std::get<string>(specifier->typeValue);
        }
        if (funDec->subNodes.size() == 3) {
            m_varTables.emplace_back();
            goto AddDefine;
        } else if (!processVarList(funDec->subNodes[2], funDef)) {
            returnNullOpt = true;
        }
        AddDefine:
        if (returnNullOpt) {
            return std::nullopt;
        }
        splDebugLog("Function header Add, Name:  " + std::get<string>(funDec->value) +
                    "\tReturn type: " + std::get<string>(specifier->value));
        insertSymbolTable(std::get<string>(funDec->value), {ValueType::FUNCTION, funDef});
        return funDef;
    }

    bool SemanticAnalyzer::processCompSt(const FunDef &funDef, NodeType &compSt) {
        bool resDefList = processDefList(compSt->subNodes[1]);
        bool resStmt = processStmtList(funDef, compSt->subNodes[2]);
        return resDefList && resStmt;  // force analyze all
    }

    bool SemanticAnalyzer::processStmtList(const FunDef &funDef, NodeType &stmtList) {
        auto *stmtNode = &stmtList;
        bool value = true;
        while (true) {
            if (static_cast<TOKEN_TYPE>((*stmtNode)->type) == TOKEN_TYPE::NOTHING) {
                break;
            }
            if (!processStmt(funDef, (*stmtNode)->subNodes[0])) {
                value = false;
            }
            stmtNode = &(*stmtNode)->subNodes[1];
        }
        return value;
    }

    bool SemanticAnalyzer::processStmt(const FunDef &funDef, NodeType &stmt) {
        if (stmt->subNodes.size() == 1 &&
            static_cast<TOKEN_TYPE>(stmt->subNodes[0]->type) == TOKEN_TYPE::NON_TERMINAL) {
            if (std::get<string>(stmt->subNodes[0]->typeValue) == "CompSt") {
                return processCompSt(funDef, stmt->subNodes[0]);
            }
            return false;
        }
        if (static_cast<TOKEN_TYPE>(stmt->subNodes[1]->type) == TOKEN_TYPE::SEMI) {
            processExp(stmt->subNodes[0]);
            return true;
        }
        switch (static_cast<TOKEN_TYPE>(stmt->subNodes[0]->type)) {
            case TOKEN_TYPE::RETURN:
                processExp(stmt->subNodes[1]);
                ValueType returnType;
                if (stmt->subNodes[1]->valueType == ValueType::ID) {
                    if (auto varNode = getVarNode(std::get<string>(stmt->subNodes[1]->value))) {
                        if (varNode->type == ValueType::ARRAY) {
                            appendError(8, stmt->loc, SEMANTIC_ERROR_TEMPLATE[7]);
                            return false;
                        } else {
                            returnType = varNode->type;
                        }
                    } else {
                        appendError(1, stmt->loc,
                                    std::get<string>(stmt->subNodes[1]->value) + SEMANTIC_ERROR_TEMPLATE[0]);
                        return false;
                    }
                } else {
                    returnType = stmt->subNodes[1]->valueType;
                }
                if (funDef.returnType != returnType) {
                    appendError(8, stmt->loc, SEMANTIC_ERROR_TEMPLATE[7]);
                    return false;
                }
                break;
            case TOKEN_TYPE::IF: {
                bool value = true;
                if (stmt->subNodes.size() == 5) {
                    processExp(stmt->subNodes[2]);
                    if (stmt->subNodes[2]->valueType != ValueType::INT) {
                        appendError(17, stmt->loc, SEMANTIC_ERROR_TEMPLATE[16]);
                        value = false;
                    }
                    m_varTables.emplace_back();
                    value = processStmt(funDef, stmt->subNodes[4]) && value;
                    m_varTables.pop_back();
                } else {
                    processExp(stmt->subNodes[2]);
                    if (stmt->subNodes[2]->valueType != ValueType::INT) {
                        appendError(17, stmt->loc, SEMANTIC_ERROR_TEMPLATE[16]);
                        value = false;
                    }
                    if (!processStmt(funDef, stmt->subNodes[4])) {
                        value = false;
                    }
                    m_varTables.emplace_back();
                    value = processStmt(funDef, stmt->subNodes[6]) && value;
                    m_varTables.pop_back();
                }
                return value;
            }
            case TOKEN_TYPE::WHILE: {
                bool value = true;
                processExp(stmt->subNodes[2]);
                if (stmt->subNodes[2]->valueType != ValueType::INT) {
                    appendError(17, stmt->loc, SEMANTIC_ERROR_TEMPLATE[16]);
                    value = false;
                }
                return processStmt(funDef, stmt->subNodes[4]) && value;
            }
            default:
                return false;
        }
        return true;
    }

    bool SemanticAnalyzer::analyze() {
        m_varTables.clear();
        m_symbolTables.clear();
        waitFun.clear();
        m_varTables.emplace_back();
        m_symbolTables.emplace_back();
        for (auto &rootNode: frontage.m_parseTree) {
            for (auto &extDefListNode: rootNode->subNodes) {
                processExtDefList(extDefListNode);
            }
        }
        return m_errors.empty();
    }

    string SemanticAnalyzer::getErrors() const {
        std::stringstream ss;
        for (const auto &error: m_errors) {
            ss << error << '\n';
        }
        return ss.str();
    }

    SymbolTableble &SemanticAnalyzer::getSymbolTable() {
        return m_symbolTables.back();
    }

    bool SemanticAnalyzer::hadDefined(const std::string &id, ValueType type) {
        auto &symbolTable = getSymbolTable();
        if (symbolTable.count(id) == 0) {
            return false;
        }
        auto &container = symbolTable[id];
        for (auto &item: container) {
            if (item.type == type) {
                return true;
            }
        }
        return false;
    }

    bool SemanticAnalyzer::hadDeclareInCurrentScope(const std::string &id) {
        return m_varTables.back().count(id) != 0;
    }

    bool SemanticAnalyzer::hadDeclareInAllScope(const std::string &id) {
        bool find = false;
        for_each(m_varTables.rbegin(), m_varTables.rend(), [&](const auto &varTable) {
            if (varTable.count(id) != 0) {
                find = true;
                return;
            }
        });
        return find;
    }

    void SemanticAnalyzer::insertSymbolTable(const string &id, const DefNode &defnode) {
        auto &table = getSymbolTable();
        if (table.count(id) == 0) {
            table[id] = {defnode};
        } else {
            table[id].push_back(defnode);
        }
    }

    std::optional<VarNode> SemanticAnalyzer::getVarNode(const std::string &id) const {
        for (auto iter = m_varTables.rbegin(); iter != m_varTables.rend(); ++iter) {
            if (iter->count(id) != 0) {
                return iter->at(id);
            }
        }
        return std::nullopt;
    }

    std::optional<DefNode> SemanticAnalyzer::getDefNode(const std::string &id, ValueType type) {
        if (hadDefined(id, type)) {
            auto &symbolTable = getSymbolTable();
            auto &container = symbolTable[id];
            for (auto &item: container) {
                if (item.type == type) {
                    return item;
                }
            }
        }
        return std::nullopt;
    }
}  // namespace spl
