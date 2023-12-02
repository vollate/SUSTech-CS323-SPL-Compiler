#include "Semantic.hpp"
#include "Frontage.hpp"
#include "Parser.hpp"

#include <any>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>

namespace spl {
    using std::string;
    namespace {
        bool isEqual(const AryDef& lhs, const AryDef& rhs) {
            if(lhs.type != rhs.type) {
                return false;
            }
            if(lhs.subAryLength.size() != rhs.subAryLength.size()) {
                return false;
            }
            auto lhsIter = lhs.subAryLength.begin();
            auto rhsIter = rhs.subAryLength.begin();
            while(lhsIter != lhs.subAryLength.end()) {
                if(*lhsIter != *rhsIter) {
                    return false;
                }
                ++lhsIter;
                ++rhsIter;
            }
            return true;
        }

        bool isEqual(const StructDef& lhs, const StructDef& rhs) {
            if(lhs.memberTypes.size() != rhs.memberTypes.size()) {
                return false;
            }
            auto lhsIter = lhs.memberTypes.begin();
            auto rhsIter = rhs.memberTypes.begin();
            size_t ary_index = 0;
            while(lhsIter != lhs.memberTypes.end()) {
                if(*lhsIter != *rhsIter) {
                    return false;
                } else if(*lhsIter == ValueType::ARRAY) {
                }
                ++lhsIter;
                ++rhsIter;
            }
            return true;
        }

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

        void updateValueType(NodeType& node) {
            node->valueType = string2ValueType(node->value);
        }

        bool isInt(const NodeType& node) {
            return node->valueType == ValueType::INT;
        }

        bool isFloat(const NodeType& node) {
            return node->valueType == ValueType::FLOAT;
        }

        bool isChar(const NodeType& node) {
            return node->valueType == ValueType::CHAR;
        }

        bool canCompare(const NodeType& lhs, const NodeType& rhs) {
            return (isInt(lhs) || isFloat(lhs) || isChar(lhs)) && (isInt(rhs) || isFloat(rhs) || isChar(rhs));
        }

        bool processArray(AryDef& aryDef, NodeType& varDec) {
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

    bool SemanticAnalyzer::canAssign(const NodeType& lhs, const NodeType& rhs) {
        switch(lhs->valueType) {
            case ValueType::INT:
            case ValueType::FLOAT:
            case ValueType::CHAR:
            case ValueType::NONE:
                return false;
            default:
                break;
        }
        if(lhs->valueType == rhs->valueType) {
            if(lhs->valueType == ValueType::ARRAY) {  // array can't assign
                return false;
            }
            if(lhs->valueType == ValueType::STRUCT) {  // struct can't be undefined since it's checked in vardec
                auto lhsVar = getVarNode(std::get<string>(lhs->value));
                auto rhsVar = getVarNode(std::get<string>(rhs->value));
                return isEqual(std::any_cast<StructDef>(lhsVar->val), std::any_cast<StructDef>(rhsVar->val));
            }
            return true;
        }
        ValueType lhsValueType = string2ValueType(lhs->value);
        ValueType rhsValueType = (rhs->valueType == ValueType::ID ? string2ValueType(rhs->value) : rhs->valueType);
        switch(lhsValueType) {
            case ValueType::FLOAT:
                switch(rhsValueType) {
                    case ValueType::FLOAT:
                    case ValueType::INT:
                    case ValueType::CHAR:
                        return true;
                    default:
                        break;
                }
                break;
            case ValueType::INT:
                switch(rhsValueType) {
                    case ValueType::INT:
                    case ValueType::CHAR:
                        return true;
                    default:
                        break;
                }
                break;
            case ValueType::CHAR:
                if(rhsValueType == ValueType::CHAR) {
                    return true;
                }
                break;
            default:
                break;
        }
        return false;
    }

    bool SemanticAnalyzer::processVarList(NodeType& varList, FunDef& funDef) {
        auto* varNode = &varList;
        m_varTables.emplace_back();
        while(true) {
            if(!processParamDec((*varNode)->subNodes[0], funDef)) {
                m_varTables.pop_back();
                return false;
            } else if((*varNode)->subNodes.size() == 1) {
                break;
            }
            varNode = &(*varNode)->subNodes[2];
        }
        return true;
    }

    bool SemanticAnalyzer::processParamDec(NodeType& paramDec, FunDef& funDef) {
        auto& specifier = paramDec->subNodes[0];
        auto& varDec = paramDec->subNodes[1];
        processSpecifier(specifier);
        if(hadDeclareInCurrentScope(std::get<string>(varDec->value))) {
            appendError(3, specifier->loc, std::get<string>(varDec->value) + SEMANTIC_ERROR_TEMPLATE[2]);
            return false;
        }
        updateValueType(specifier);
        splDebugLog("Parameter Add, Name:  " + std::get<string>(varDec->value) + "\tType: " + std::get<string>(specifier->value));
        if(isInt(specifier) || isFloat(specifier) || isChar(specifier)) {  // primary type & array
            if(varDec->subNodes.size() == 1) {                             //  primary type
                funDef.argTypes.push_back(specifier->valueType);
                declareVariable(specifier, varDec);
            } else {  //  array
                AryDef aryDef{ specifier->valueType };
                if(!processArray(aryDef, varDec)) {
                    return false;
                }
                funDef.argTypes.push_back(ValueType::ARRAY);
                funDef.argAry.emplace_back(funDef.argTypes.size() - 1, aryDef);
                declareVariable(specifier, varDec);
            }
        } else {  // struct
            if(!hadDefined(std::get<string>(specifier->value), ValueType::STRUCT)) {
                appendError(16, specifier->loc, SEMANTIC_ERROR_TEMPLATE[15]);
                return false;
            }
            funDef.argTypes.push_back(ValueType::STRUCT);
            declareVariable(specifier, varDec);
        }
        funDef.argIds.push_back(std::get<string>(varDec->value));
        return true;
    }

    bool SemanticAnalyzer::processVarDec(NodeType& specifier, NodeType& varDec, bool structDef) {
        if(hadDeclareInCurrentScope(std::get<string>(varDec->value))) {
            appendError(3, specifier->loc, std::get<string>(varDec->value) + SEMANTIC_ERROR_TEMPLATE[2]);
            return false;
        }
        auto storeSpec = specifier->value;
        updateValueType(specifier);
        if(isInt(specifier) || isFloat(specifier) || isChar(specifier)) {  // primary type & array
            if(varDec->subNodes.size() == 1) {                             //  primary type
                if(!declareVariable(specifier, varDec)) {
                    if(!structDef)
                        varDec->value = storeSpec;
                    return false;
                }
                if(!structDef)
                    varDec->value = storeSpec;
            } else {  //  array
                AryDef aryDef{ specifier->valueType };
                if(!processArray(aryDef, varDec)) {
                    return false;
                }
                if(!declareVariable(specifier, varDec)) {
                    return false;
                }
            }
        } else {  // struct, useless since no struct before id in the induction
            //            return false;
            if(!structDef)
                varDec->value = storeSpec;
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

    void SemanticAnalyzer::processExtDef(NodeType& extDef) {
        processSpecifier(extDef->subNodes[0]);
        if(extDef->subNodes.size() == 2) {  // extDef -> specifier SEMI
            return;
        } else if(static_cast<TOKEN_TYPE>(extDef->subNodes[2]->type) ==
                  TOKEN_TYPE::SEMI) {  // extDef -> specifier extDecList SEMI, process with iteration
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
            if(auto funDef = processFunDec(extDef->subNodes[0], extDef->subNodes[1])) {
                if(!processCompSt(funDef.value(), extDef->subNodes[2])) {
                    splDebugLog("Function " + std::get<string>(extDef->subNodes[1]->value) + " has error, remove it");
                    getSymbolTable().erase(
                        std::get<string>(extDef->subNodes[0]->value));  // remove error function from symbol table
                }
            }
            m_varTables.pop_back();
        }
    }

    bool SemanticAnalyzer::processDec(StructDef& structDef, NodeType& specifier, NodeType& dec) {  // for struct def
        if(!processVarDec(specifier, dec->subNodes[0], true)) {
            return false;
        }
        structDef.memberTypes.push_back(specifier->valueType);
        structDef.memberIds.push_back(std::get<string>(dec->subNodes[0]->value));
        if(dec->subNodes.size() == 1) {  // dec -> varDec
            return true;
        }
        // dec -> varDec ASSIGN exp
        processExp(dec->subNodes[2]);
        if(!canAssign(dec->subNodes[0], dec->subNodes[2])) {
            appendError(5, dec->loc, SEMANTIC_ERROR_TEMPLATE[4]);
            return false;
        }
        return true;
    }

    bool SemanticAnalyzer::processDec(NodeType& specifier, NodeType& dec) {
        if(!processVarDec(specifier, dec->subNodes[0])) {
            return false;
        }
        if(dec->subNodes.size() == 1) {  // dec -> varDec
            return true;
        }
        // dec -> varDec ASSIGN exp
        processExp(dec->subNodes[2]);
        if(!canAssign(dec->subNodes[0], dec->subNodes[2])) {
            appendError(5, dec->loc, SEMANTIC_ERROR_TEMPLATE[4]);
            return false;
        }
        return true;
    }

    void SemanticAnalyzer::processExp(NodeType& exp) {
        auto& subNodes = exp->subNodes;
        if(subNodes.size() == 1) {
            if(static_cast<TOKEN_TYPE>(exp->subNodes[0]->type) == TOKEN_TYPE::ID) {
                if(auto varNode = getVarNode(std::get<string>(subNodes[0]->value))) {
                    switch(varNode->type) {
                        case ValueType::ARRAY:
                            exp->valueType = ValueType::ARRAY;
                            break;
                        case ValueType::STRUCT:
                            exp->valueType = ValueType::STRUCT;
                            break;
                        default:
                            break;
                    }
                } else {
                    appendError(1, exp->loc, std::get<string>(exp->subNodes[0]->value) + SEMANTIC_ERROR_TEMPLATE[0]);
                }
            }
        } else if(subNodes.size() == 2) {  //+/- exp
            processExp(subNodes[1]);
            if(static_cast<TOKEN_TYPE>(subNodes[0]->type) == TOKEN_TYPE::NOT && (subNodes[1]->valueType != ValueType::INT)) {
                appendError(7, exp->loc, SEMANTIC_ERROR_TEMPLATE[6]);
            }
            if(!(isInt(subNodes[1]) || !isFloat(subNodes[1]))) {
                appendError(7, exp->loc, SEMANTIC_ERROR_TEMPLATE[6]);
            }
            exp->valueType = exp->subNodes[1]->valueType;
        } else if(subNodes.size() == 3) {
            if(static_cast<TOKEN_TYPE>(subNodes[0]->type) == TOKEN_TYPE::LP) {
                processExp(subNodes[2]);
                exp->valueType = subNodes[2]->valueType;
                exp->value = subNodes[2]->value;
                return;
            } else if(subNodes[0]->valueType == ValueType::ID) {  // deal function call
                if(auto defNode = getDefNode(std::get<string>(subNodes[0]->value), ValueType::FUNCTION)) {
                    auto funDef = std::get<FunDef>(defNode->val);
                    if(!funDef.argIds.empty()) {
                        appendError(9, exp->loc,
                                    SEMANTIC_ERROR_TEMPLATE[8] + std::to_string(funDef.argIds.size()) + " but got 0");
                    } else {
                        exp->valueType = funDef.returnType;
                    }
                    return;
                } else {
                    appendError(2, exp->loc, std::get<string>(subNodes[0]->value) + SEMANTIC_ERROR_TEMPLATE[1]);
                }
            }
            processExp(subNodes[0]);
            processExp(subNodes[2]);
            switch(static_cast<TOKEN_TYPE>(subNodes[1]->type)) {
                case TOKEN_TYPE::ASSIGN: {
                    if(!canAssign(subNodes[0], subNodes[2])) {
                        appendError(5, exp->loc, SEMANTIC_ERROR_TEMPLATE[4]);
                    } else {
                        exp->valueType = subNodes[0]->valueType;
                    }
                    break;
                }
                    // logic
                case TOKEN_TYPE::AND:
                case TOKEN_TYPE::OR:
                    if(!isInt(subNodes[0]) || !isInt(subNodes[2])) {
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
                    if(!canCompare(subNodes[0], subNodes[2])) {
                        appendError(17, exp->loc, SEMANTIC_ERROR_TEMPLATE[6]);
                    } else {
                        exp->valueType = ValueType::INT;
                    }
                    break;
                    // arthimetic
                case TOKEN_TYPE::PLUS:
                case TOKEN_TYPE::MINUS:
                case TOKEN_TYPE::MUL:
                case TOKEN_TYPE::DIV:
                    // TODO
                    if(!isInt(subNodes[0]) || !isInt(subNodes[2])) {
                        appendError(7, exp->loc, SEMANTIC_ERROR_TEMPLATE[6]);
                    } else {
                        exp->valueType = ValueType::INT;
                    }
                    break;
                default:
                    break;
            }
        } else if(subNodes.size() == 4) {  // Array Check
            // Function Call
            // TODO
        }
    }

    bool processArgs(NodeType& args) {
        // TODO
        return true;
    }

    bool SemanticAnalyzer::processSpecifier(NodeType& specifier) {
        if(specifier->valueType != ValueType::STRUCT) {
            updateValueType(specifier);
            return true;
        }
        return processStructSpecifier(specifier->subNodes[0]);
    }

    bool SemanticAnalyzer::processStructSpecifier(NodeType& structSpecifier) {
        if(hadDefined(std::get<string>(structSpecifier->value), ValueType::STRUCT)) {
            if(structSpecifier->subNodes.size() != 2) {
                appendError(4, structSpecifier->loc, SEMANTIC_ERROR_TEMPLATE[3]);
                return false;
            }
        } else {
            if(structSpecifier->subNodes.size() == 2) {
                appendError(16, structSpecifier->loc, SEMANTIC_ERROR_TEMPLATE[15]);
                return false;
            }
            StructDef structDef;
            if(!processDefList(structDef, structSpecifier->subNodes[3])) {
                return false;
            }
            insertSymbolTable(std::get<string>(structSpecifier->value), { ValueType::STRUCT, structDef });
        }
        return true;
    }

    bool SemanticAnalyzer::processDecList(NodeType& specifier, NodeType& decList) {
        auto* decListNode = &decList;
        bool value = true;
        while(true) {
            if(!processDec(specifier, (*decListNode)->subNodes[0])) {
                // return false;
                value = false;
            }
            if((*decListNode)->subNodes.size() == 1) {
                break;
            }
            decListNode = &(*decListNode)->subNodes[2];
        }
        return value;
    }

    bool SemanticAnalyzer::processDecList(StructDef& structDef, NodeType& specifier, NodeType& decList) {
        auto* decListNode = &decList;
        bool value = true;
        while(true) {
            if(!processDec(structDef, specifier, (*decListNode)->subNodes[0])) {
                //                return false;
                value = false;
            }
            if((*decListNode)->subNodes.size() == 1) {
                break;
            }
            decListNode = &(*decListNode)->subNodes[2];
        }
        return value;
    }

    bool SemanticAnalyzer::processDef(NodeType& def) {  // iterate the declist
        bool value = processSpecifier(def->subNodes[0]);
        return processDecList(def->subNodes[0], def->subNodes[1]) && value;
    }

    bool SemanticAnalyzer::processDef(StructDef& stuctDef, NodeType& def) {  // iterate the DecLit, for struct definition
        bool value = processSpecifier(def->subNodes[0]);
        return processDecList(stuctDef, def->subNodes[0], def->subNodes[1]) && value;
    }

    bool SemanticAnalyzer::processDefList(NodeType& defList) {
        auto* defListNode = &defList;
        bool value = true;
        while(true) {
            if(static_cast<TOKEN_TYPE>((*defListNode)->type) == TOKEN_TYPE::NOTHING) {
                break;
            }
            if(!processDef((*defListNode)->subNodes[0])) {
                // return false;
                value = false;  // force analyze all
            }
            defListNode = &(*defListNode)->subNodes[1];
        }
        return value;
    }

    bool SemanticAnalyzer::processDefList(StructDef& structDef, NodeType& defList) {
        m_varTables.emplace_back();
        auto* defListNode = &defList;
        bool value = true;
        while(true) {
            if(static_cast<TOKEN_TYPE>((*defListNode)->type) == TOKEN_TYPE::NOTHING) {
                break;
            }
            if(!processDef(structDef, (*defListNode)->subNodes[0])) {
                // return false;
                value = false;
            }
            if((*defListNode)->subNodes.size() == 1) {
                break;
            }
            defListNode = &(*defListNode)->subNodes[1];
        }
        m_varTables.pop_back();
        return value;
    }

    bool SemanticAnalyzer::declareVariable(NodeType& specifier, const NodeType& var) {
        auto& varTable = m_varTables.back();
        if(hadDeclareInCurrentScope(std::get<string>(var->value))) {
            appendError(3, specifier->loc, std::get<string>(var->value) + SEMANTIC_ERROR_TEMPLATE[2]);
            return false;
        }
        splDebugLog("Variable Add, Name:  " + std::get<string>(var->value) + "\tType: " + std::get<string>(specifier->value));
        switch(specifier->valueType) {
            case ValueType::INT:
            case ValueType::FLOAT:
            case ValueType::CHAR:
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

    void SemanticAnalyzer::processExtDefList(NodeType& extDefList) {
        while(true) {
            switch(static_cast<TOKEN_TYPE>(extDefList->type)) {
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

    std::optional<FunDef> SemanticAnalyzer::processFunDec(NodeType& specifier, NodeType& funDec) {
        bool returnNullOpt = false;
        if(hadDefined(std::get<string>(funDec->value), ValueType::FUNCTION)) {
            appendError(4, specifier->loc, "Function " + std::get<string>(funDec->value) + SEMANTIC_ERROR_TEMPLATE[3]);
            return std::nullopt;
        }
        FunDef funDef;
        if(specifier->valueType != ValueType::STRUCT) {
            funDef.returnType = string2ValueType(specifier->value);
            funDef.returnTypeName = std::get<string>(specifier->value);
        } else {
            if(!hadDefined(std::get<string>(specifier->value), ValueType::STRUCT)) {
                appendError(16, specifier->loc, SEMANTIC_ERROR_TEMPLATE[15]);
                returnNullOpt = true;
            }
            funDef.returnType = ValueType::STRUCT;
            funDef.returnTypeName = std::get<string>(specifier->typeValue);
        }
        if(funDec->subNodes.size() == 3) {
            m_varTables.emplace_back();
            goto AddDefine;
        } else if(!processVarList(funDec->subNodes[2], funDef)) {
            returnNullOpt = true;
        }
    AddDefine:
        if(returnNullOpt) {
            return std::nullopt;
        }
        splDebugLog("Function header Add, Name:  " + std::get<string>(funDec->value) +
                    "\tReturn type: " + std::get<string>(specifier->value));
        insertSymbolTable(std::get<string>(funDec->value), { ValueType::FUNCTION, funDef });
        return funDef;
    }

    bool SemanticAnalyzer::processCompSt(const FunDef& funDef, NodeType& compSt) {
        bool resDefList = processDefList(compSt->subNodes[1]);
        bool resStmt = processStmtList(funDef, compSt->subNodes[2]);
        return resDefList && resStmt;  // force analyze all
    }

    bool SemanticAnalyzer::processStmtList(const FunDef& funDef, NodeType& stmtList) {
        auto* stmtNode = &stmtList;
        bool value = true;
        while(true) {
            if((*stmtNode)->subNodes.size() == 1) {
                break;
            }
            if(!processStmt(funDef, (*stmtNode)->subNodes[0])) {
                value = false;
            }
            stmtNode = &(*stmtNode)->subNodes[1];
        }
        return value;
    }

    bool SemanticAnalyzer::processStmt(const FunDef& funDef, NodeType& stmt) {
        if(static_cast<TOKEN_TYPE>(stmt->subNodes[1]->type) == TOKEN_TYPE::SEMI) {
            processExp(stmt->subNodes[0]);
            return true;
        }
        if(static_cast<TOKEN_TYPE>(stmt->subNodes[0]->type) == TOKEN_TYPE::NON_TERMINAL) {
            if(std::get<string>(stmt->subNodes[0]->typeValue) == "CompSt") {
                return processCompSt(funDef, stmt->subNodes[0]);
            }
            return false;
        }
        switch(static_cast<TOKEN_TYPE>(stmt->subNodes[0]->type)) {
            case TOKEN_TYPE::RETURN:
                processExp(stmt->subNodes[1]);
                if(funDef.returnType != stmt->subNodes[1]->valueType) {
                    appendError(8, stmt->loc, SEMANTIC_ERROR_TEMPLATE[7]);
                    return false;
                }
            case TOKEN_TYPE::IF: {
                bool value = true;
                if(stmt->subNodes.size() == 5) {
                    processExp(stmt->subNodes[2]);
                    if(stmt->subNodes[2]->valueType != ValueType::INT) {
                        appendError(17, stmt->loc, SEMANTIC_ERROR_TEMPLATE[16]);
                        value = false;
                    }
                    return processStmt(funDef, stmt->subNodes[4]) && value;
                } else {
                    processExp(stmt->subNodes[2]);
                    if(stmt->subNodes[2]->valueType != ValueType::INT) {
                        appendError(17, stmt->loc, SEMANTIC_ERROR_TEMPLATE[16]);
                        value = false;
                    }
                    if(!processStmt(funDef, stmt->subNodes[4])) {
                        value = false;
                    }
                    return processStmt(funDef, stmt->subNodes[6]) && value;
                }
            }
            case TOKEN_TYPE::WHILE: {
                bool value = true;
                processExp(stmt->subNodes[2]);
                if(stmt->subNodes[2]->valueType != ValueType::INT) {
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
        for(auto& rootNode : frontage.m_parseTree) {
            for(auto& extDefListNode : rootNode->subNodes) {
                processExtDefList(extDefListNode);
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
        if(symbolTable.count(id) == 0) {
            return false;
        }
        auto& container = symbolTable[id];
        for(auto& item : container) {
            if(item.type == type) {
                return true;
            }
        }
        return false;
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

    void SemanticAnalyzer::insertSymbolTable(const string& id, const DefNode& defnode) {
        auto& table = getSymbolTable();
        if(table.count(id) == 0) {
            table[id] = { defnode };
        } else {
            table[id].push_back(defnode);
        }
    }

    std::optional<VarNode> SemanticAnalyzer::getVarNode(const std::string& id) const {
        for(auto iter = m_varTables.rbegin(); iter != m_varTables.rend(); ++iter) {
            if(iter->count(id) != 0) {
                return iter->at(id);
            }
        }
        return std::nullopt;
    }

    std::optional<DefNode> SemanticAnalyzer::getDefNode(const std::string& id, ValueType type) {
        if(hadDefined(id, type)) {
            auto& symbolTable = getSymbolTable();
            auto& container = symbolTable[id];
            for(auto& item : container) {
                if(item.type == type) {
                    return item;
                }
            }
        }
        return std::nullopt;
    }
}  // namespace spl
