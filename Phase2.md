# Phase 2

## Requirements

- [ ] Type 1: a variable is used without a definition
- [ ] Type 2: a function is invoked without a definition
- [x] Type 3: a variable is redefined in the same scope
- [x] Type 4: a function is redefined (in the global scope, since we don’t have nested functions)
- [ ] Type 5: unmatching types appear at both sides of the assignment operator (=)
- [ ] Type 6: rvalue appears on the left-hand side of the assignment operator
- [ ] Type 7: unmatching operands, such as adding an integer to a structure variable
- [ ] Type 8: a function’s return value type mismatches the declared type
- [ ] Type 9: a function’s arguments mismatch the declared parameters (either types or numbers, or both)
- [ ] Type 10: applying indexing operator ([...]) on non-array type variables
- [ ] Type 11: applying function invocation operator (foo(...)) on non-function names
- [ ] Type 12: array indexing with a non-integer type expression
- [ ] Type 13: accessing members of a non-structure variable (i.e., misuse the dot operator)
- [ ] Type 14: accessing an undefined structure member
- [x] Type 15: redefine the same structure type
- [x] Type 16: use struct without define or declare

## Test Cases status
- [ ] test_2_r01.spl
- [ ] test_2_r02.spl
- [x] test_2_r03.spl
- [ ] test_2_r04.spl
- [ ] test_2_r05.spl
- [ ] test_2_r06.spl
- [ ] test_2_r07.spl
- [ ] test_2_r08.spl
- [ ] test_2_r09.spl
- [ ] test_2_r10.spl
- [ ] test_2_r11.spl
- [ ] test_2_r12.spl
- [ ] test_2_r13.spl
- [ ] test_2_r14.spl
- [ ] test_2_r15.spl