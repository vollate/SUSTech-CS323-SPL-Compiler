find_package(BISON REQUIRED)
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    BISON_TARGET(bison_parser Parser.y ${CMAKE_PRECOMPILE_OUTPUT_DIR}/Parser.cpp
#            COMPILE_FLAGS "--debug -Wcounterexamples"
            DEFINES_FILE ${CMAKE_PRECOMPILE_OUTPUT_DIR}/Parser.hpp)
else ()
    BISON_TARGET(bison_parser Parser.y ${CMAKE_PRECOMPILE_OUTPUT_DIR}/Parser.cpp
            DEFINES_FILE ${CMAKE_PRECOMPILE_OUTPUT_DIR}/Parser.hpp)
endif ()
add_library(parser ${BISON_bison_parser_OUTPUTS})