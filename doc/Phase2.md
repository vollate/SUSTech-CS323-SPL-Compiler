# Phase 2

## Requirements

- [x] Type 1: a variable is used without a definition
- [x] Type 2: a function is invoked without a definition
- [x] Type 3: a variable is redefined in the same scope
- [x] Type 4: a function is redefined (in the global scope, since we don’t have nested functions)
- [x] Type 5: unmatching types appear at both sides of the assignment operator (=)
- [x] Type 6: rvalue appears on the left-hand side of the assignment operator
- [x] Type 7: unmatching operands, such as adding an integer to a structure variable
- [x] Type 8: a function’s return value type mismatches the declared type
- [x] Type 9: a function’s arguments mismatch the declared parameters (either types or numbers, or both)
- [x] Type 10: applying indexing operator ([...]) on non-array type variables
- [x] Type 11: applying function invocation operator (foo(...)) on non-function names
- [x] Type 12: array indexing with a non-integer type expression
- [x] Type 13: accessing members of a non-structure variable (i.e., misuse the dot operator)
- [x] Type 14: accessing an undefined structure member
- [x] Type 15: redefine the same structure type
- [x] Type 16: use struct without define or declare
- [x] Type 17: unmatching type on both sides of logic operation (only int can do logic operation)
- [x] Type 18: unmatching type on both sides of compare operation (only int/float/char can do compare operation)
- [x] Type 19: unmatch type for function argument(s)

支持 scoop variable
## Test Cases status

- [x] test_2_r01.spl
- [x] test_2_r02.spl
- [x] test_2_r03.spl
- [x] test_2_r04.spl
- [x] test_2_r06.spl
- [x] test_2_r07.spl
- [x] test_2_r08.spl
- [x] test_2_r09.spl
- [x] test_2_r10.spl
- [x] test_2_r11.spl
- [x] test_2_r12.spl
- [ ] test_2_r13.spl (忘考虑成员变量是struct的情况了，写完太难改了，不看样例导致的，有时间再重构)
- [x] test_2_r14.spl
- [x] test_2_r15.spl

