add_executable(lexer_tester "LexerTest.cpp")
#target_link_libraries(lexer_tester PRIVATE scanner)
target_link_libraries(lexer_tester PRIVATE spl_frontage)


foreach(i RANGE 1 5)
  add_test(NAME lexer_testcase${i}
           COMMAND $<TARGET_FILE:lexer_tester>
          ${CMAKE_SOURCE_DIR}/test/testcases/testcase${i}.txt)
endforeach()
