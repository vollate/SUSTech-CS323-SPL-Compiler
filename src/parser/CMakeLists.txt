find_package(BISON REQUIRED)
BISON_TARGET(bison_parser Parser.y ${CMAKE_PRECOMPILE_OUTPUT_DIR}/Parser.cpp
        DEFINES_FILE ${CMAKE_PRECOMPILE_OUTPUT_DIR}/Parser.hpp)
add_library(parser ${BISON_bison_parser_OUTPUTS})