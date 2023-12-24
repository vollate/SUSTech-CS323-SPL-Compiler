#include "optimize.h"
#include <regex.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 100

static void debugInfo(const char* pattern, ...) {
#ifdef OPTIMIZER_DEBUG
    va_list args;
    va_start(args, pattern);
    vprintf(pattern, args);
    va_end(args);
#endif
}

static void debugError(const char* pattern, ...) {
#ifdef OPTIMIZER_DEBUG
    va_list args;
    va_start(args, pattern);
    vfprintf(stderr, pattern, args);
    va_end(args);
#endif
}

typedef struct VarNode {
    char* name;
    int value;
    bool constant;
    struct VarNode* next;
} VarNode;

typedef struct LabelNode {
    char* name;
    size_t lineNumber;
    struct LabelNode* next;
} LabelNode;

static bool read_line(FILE* file, char* line) {
    if (fgets(line, MAX_LINE_LENGTH, file) != NULL)
    {
        line[strlen(line) - 1] = '\0';
        return true;
    }else if(feof(file)) {
        printf("read line eof\n");
    }
    return false;
}

static VarNode* addVar(VarNode* head, const char* name, int value, bool constant) {
    while(head->next != NULL) {
        if(!strcmp(head->name, name)) {
            return head;
        }
        head = head->next;
    }
    VarNode* node = (VarNode*)malloc(sizeof(VarNode));
    if(node == NULL) {
        debugError("malloc error in add var\n");
        return NULL;
    }
    node->name = (char*)malloc(strlen(name) + 1);
    memcpy(node->name, name, strlen(name) + 1);
    node->value = 0;
    node->constant = constant;
    node->next = NULL;
    head->next = node;
    return node;
}

static LabelNode* addLabel(LabelNode* head, const char* name, size_t lineNumber) {
    LabelNode* node = (LabelNode*)malloc(sizeof(LabelNode));
    if(node == NULL) {
        debugError("malloc error in add label\n");
        return NULL;
    }
    node->name = (char*)malloc(strlen(name) + 1);
    memcpy(node->name, name, strlen(name) + 1);
    node->lineNumber = lineNumber;
    node->next = NULL;
    while(head->next != NULL) {
        head = head->next;
    }
    head->next = node;
    return node;
}

static VarNode* findVar(VarNode* head, const char* name) {
    while(head != NULL) {
        if(!strcmp(head->name, name)) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

static LabelNode* findLabel(LabelNode* head, const char* name) {
    while(head != NULL) {
        if(!strcmp(head->name, name)) {
            return head;
        }
        head = head->next;
    }
    return NULL;
}

static void cutLabel(const char* origin, char* target) {
    int i;
    for(i = 0;; ++i) {
        if(origin[i] != ' ') {
            break;
        }
    }
    const char* begin = origin + i;
    for(i = 0;; ++i) {
        if(begin[i] != ' ' && begin[i] != ':')
            target[i] = begin[i];
        else {
            target[i] = '\0';
            break;
        }
    }
}

static bool prefixCheck(const char* prefix, const char* target) {
    return !strncmp(prefix, target, strlen(prefix));
}

static void ParseSingleVar(const char* exp, VarNode* varHead, VarNode** result) {
    if(prefixCheck("CALL", exp)) {
        result[0] = addVar(varHead, "__CALL__", 0, false);
        result[0]->constant = false;
    } else {
        if(exp[0] == '#') {
            *result = (VarNode*)malloc(sizeof(VarNode));
            (*result)->name = (char*)malloc(strlen(exp) + 1);
            memcpy((*result)->name, exp, strlen(exp) + 1);
            result[0]->value = atoi(exp + 1);
            result[0]->constant = true;
        } else {
            result[0] = findVar(varHead, exp);
        }
        if(result[0] == NULL) {
            debugInfo("Warning: use undefined variable: %s\n", exp);
            result[0] = addVar(varHead, exp, 0, false);
        }
    }
}

enum ExpType { ADD, SUB, MUL, DIV, NONE };

static VarNode** parseRhsExp(const char* exp, VarNode* varHead, enum ExpType* type) {
    regex_t regex;
    regmatch_t match[1];
    VarNode** result = (VarNode**)malloc(sizeof(VarNode*) * 2);
    result[0] = NULL;
    result[1] = NULL;
    int ret = regcomp(&regex, " [\\+-\\*/] ", 0);
    if(ret) {
        debugError("Compile regex error\n");
        return result;
    }
    ret = regexec(&regex, exp, 1, match, 0);
    if(ret) {  // single rhs
        ParseSingleVar(exp, varHead, result);
    } else {  // exp rhs
        /*debugInfo("parse rhs succeed: %s \t %d %d\n", exp, match->rm_so, match->rm_eo);*/
        switch(exp[match->rm_so + 1]) {
            case '+':
                *type = ADD;
                break;
            case '-':
                *type = SUB;
                break;
            case '*':
                *type = MUL;
                break;
            case '/':
                *type = DIV;
                break;
            default:
                debugError("parse rhs operator: %s\n", exp + match->rm_so + 1);
                break;
        }
        char tmpExp[MAX_LINE_LENGTH];
        memcpy(tmpExp, exp, match->rm_so);
        tmpExp[match->rm_so] = '\0';
        ParseSingleVar(tmpExp, varHead, result);
        ParseSingleVar(exp + match->rm_eo, varHead, result + 1);
    }
    return result;
}

static void parseLhsExp(const char* exp, VarNode* varHead, VarNode** rhs, enum ExpType opType) {
    VarNode* lhs = addVar(varHead, exp, 0, false);
    lhs->constant = false;
    return;
    if (opType == NONE)
    {
        if((lhs->constant = rhs[0]->constant)) {
            lhs->value = rhs[0]->value;
        }
    }
    else
    {
        if(rhs[0]->constant && rhs[1]->constant) {
            lhs->constant = true;
            switch(opType) {
                case ADD:
                    lhs->value = rhs[0]->value + rhs[1]->value;
                    break;
                case SUB:
                    lhs->value = rhs[0]->value - rhs[1]->value;
                    break;
                case MUL:
                    lhs->value = rhs[0]->value * rhs[1]->value;
                    break;
                case DIV:
                    lhs->value = rhs[0]->value / rhs[1]->value;
                    break;
                default:
                    break;
            }
        }
    }
    if(lhs->constant) {
        printf("Constant: %s = %d\n", lhs->name, lhs->value);
    }
}

static void parseAssign(const char* exp, VarNode* varHead) {
    regex_t regex;
    regmatch_t match[1];
    int ret = regcomp(&regex, " := ", 0);
    if(ret) {
        debugError("Compile regex error\n");
        return;
    }
    ret = regexec(&regex, exp, 1, match, 0);
    if(ret) {
        debugError("parse equation error: %s\n", exp);
    } else {
        enum ExpType opType = NONE;
        VarNode** rhs = parseRhsExp(exp + match->rm_eo, varHead, &opType);
        char tmpExp[MAX_LINE_LENGTH];
        memcpy(tmpExp, exp, match->rm_so);
        tmpExp[match->rm_so] = '\0';
        parseLhsExp(tmpExp, varHead, rhs, opType);
        free(rhs);
    }
}
static bool processIF(const char* exp, VarNode* varHead, LabelNode* labelHead, FILE* outputFile, bool* isUnreachable) {
    regex_t regex;
    regmatch_t match[1];
    int ret = regcomp(&regex, " [><=!]= ", 0);
    if(ret) {
        debugError("Compile regex error\n");
        return false;
    }
    ret = regexec(&regex, exp, 1, match, 0);
    if(ret) {
        debugError("parse IF error: %s\n", exp);
    } else {
        VarNode *lhsPtr, *rhsPtr;
        char tmpLhs[MAX_LINE_LENGTH], tmpRhs[MAX_LINE_LENGTH];
        memcpy(tmpLhs, exp, match->rm_so);
        tmpLhs[match->rm_so] = '\0';
        for(int i = match->rm_eo; i < strlen(exp); ++i) {
            if(exp[i] == ' ') {
                tmpRhs[i - match->rm_eo] = '\0';
                break;
            }
            tmpRhs[i - match->rm_eo] = exp[i];
        }
        ParseSingleVar(tmpLhs, varHead, &lhsPtr);
        ParseSingleVar(tmpRhs, varHead, &rhsPtr);
        if(!lhsPtr->constant || !rhsPtr->constant) {
            return false;
        }
        switch (exp[match->rm_so + 1]) {
            case '=':
                if (lhsPtr->value != rhsPtr->value) return false;
                break;
            case '!':
                if (lhsPtr->value == rhsPtr->value) return false;
                break;
            case '>':
                if (lhsPtr->value <= rhsPtr->value) return false;
                break;
            case '<':
                if (lhsPtr->value >= rhsPtr->value) return false;
                break;
        }
        printf("Constant if: %s %s %s\n", lhsPtr->name, exp + match->rm_so + 1, rhsPtr->name);
        ret = regcomp(&regex, " GOTO ", 0);
        if(ret) {
            debugError("Compile regex error\n");
            return false;
        }
        ret = regexec(&regex, exp, 1, match, 0);
        if(ret) {
            debugError("parse IF error: %s\n", exp);
        } else {
            fprintf(outputFile, "GOTO %s\n", exp + match->rm_eo);
            if(findLabel(labelHead, exp + match->rm_eo) == NULL)
                *isUnreachable = true;
            return true;
        }
    }
    return false;
}

void optimize(const char* inPath, const char* outPath) {
    printf("input: %s\noutput: %s\n", inPath, outPath);
    FILE *inputFile=fopen(inPath,"r"), *outputFile = fopen(outPath, "w");
    if (inputFile == NULL) {
        printf("Can't open input file: %s\n", inPath);
        return;
    } else if (outputFile == NULL) {
        printf("Can't open or create output file: %s\n", outPath);
        return;
    }

    char readBuf[MAX_LINE_LENGTH], tmpBuf[MAX_LINE_LENGTH];
    bool isUnreachable;
    isUnreachable = false;
    size_t lineNumber = 1;
    VarNode* varHead = (VarNode*)malloc(sizeof(VarNode));
    LabelNode* labelHead = (LabelNode*)malloc(sizeof(LabelNode));
    varHead->name = (char*)malloc(1);
    labelHead->name = (char*)malloc(1);
    varHead->name[0] = '\0';
    labelHead->name[0] = '\0';
    varHead->next = NULL;
    labelHead->next = NULL;
    while (read_line(inputFile, readBuf))
    {
        ++lineNumber;
        if(':' == readBuf[strlen(readBuf) - 1]) {
            if(prefixCheck("FUNCTION", readBuf)) {
                debugInfo("Define function: %s\n", readBuf + strlen("FUNCTION"));
            } else if(prefixCheck("LABEL", readBuf)) {
                isUnreachable = false;
                cutLabel(readBuf + strlen("LABEL"), tmpBuf);
                LabelNode* label = addLabel(labelHead, tmpBuf, lineNumber);
                debugInfo("Define label: %s\n", label->name);
            }
        } else {
            if(isUnreachable) {
                debugInfo("Unreachable code: %s\n", readBuf);
                goto NO_PRINT;
            }
#define PREFIX_SIZE 8
            const char prefix[PREFIX_SIZE][10] = { "READ", "WRITE", "IF", "GOTO", "PARAM", "RETURN", "ARG", "CALL" };
            int index = -1;
            for(int i = 0; i < PREFIX_SIZE; ++i) {
                if(prefixCheck(prefix[i], readBuf)) {
                    index = i;
                    break;
                }
            }
            switch(index) {
                case 0:
                    debugInfo("Read: %s\n", readBuf + strlen("READ"));
                    break;
                case 1:
                    debugInfo("Write: %s\n", readBuf + strlen("WRITE"));
                    break;
                case 2:
                    if(processIF(readBuf + 3, varHead, labelHead, outputFile, &isUnreachable)) {
                        goto NO_PRINT;
                    }
                    break;
                case 3:
                    debugInfo("%s\n", readBuf);
                    /*if(findLabel(labelHead, readBuf + 5) == NULL)*/
                        isUnreachable = true;
                    break;
                case 4:
                    debugInfo("Add param %s\n", readBuf + 6);
                    addVar(varHead, readBuf + 6, 0, false);
                    break;
                case 5:
                    debugInfo("%s\n", readBuf);
                    break;
                case 6:
                    debugInfo("Push args: %s\n", readBuf + 4);
                    break;
                case 7:
                    debugInfo("%s\n", readBuf);
                    break;
                default:
                    if(strlen(readBuf) == 0)
                        debugInfo("Empty line\n");
                    else
                        parseAssign(readBuf, varHead);
            }
        }
        fprintf(outputFile, "%s\n", readBuf);
        ++lineNumber;
    NO_PRINT:
        continue;
    }
    fclose(outputFile);
    while(varHead != NULL) {
        VarNode* tmp = varHead;
        varHead = varHead->next;
        free(tmp->name);
        free(tmp);
    }
    while(labelHead != NULL) {
        LabelNode* tmp = labelHead;
        labelHead = labelHead->next;
        free(tmp);
    }
}

// int main() {
//     optimize("test.txt", "test.out");
//     return 0;
// }
