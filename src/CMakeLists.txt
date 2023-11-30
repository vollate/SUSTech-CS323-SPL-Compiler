add_subdirectory(scanner)
add_subdirectory(parser)
add_subdirectory(semantic_analyzer)

add_library(spl_frontage "./Frontage.cpp" )
target_link_libraries(spl_frontage PRIVATE scanner)
target_link_libraries(spl_frontage PRIVATE parser)
target_link_libraries(spl_frontage PRIVATE semantic_analyzer)