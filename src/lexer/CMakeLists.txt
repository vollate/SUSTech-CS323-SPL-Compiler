find_package(FLEX REQUIRED)
flex_target(flex_analyzer Lexer.ll
            ${CMAKE_CURRENT_BINARY_DIR}/Lexer.cpp)
add_library(lexer ${FLEX_flex_analyzer_OUTPUTS})
