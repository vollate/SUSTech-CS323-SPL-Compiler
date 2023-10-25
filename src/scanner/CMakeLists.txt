find_package(FLEX REQUIRED)
flex_target(flex_analyzer Scanner.l
            ${CMAKE_PRECOMPILE_OUTPUT_DIR}/Scanner.cpp)
add_library(scanner ${FLEX_flex_analyzer_OUTPUTS})
