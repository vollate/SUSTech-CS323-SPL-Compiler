add_executable(phase1_tester "Phase1Tester.cpp")
target_link_libraries(phase1_tester PRIVATE spl_frontage)

foreach(i RANGE 1 9)
  add_test(NAME phase1_testcase${i}
           COMMAND $<TARGET_FILE:phase1_tester>
                   ${CMAKE_SOURCE_DIR}/test/phase1/testcases/test_1_r0${i}.spl)
endforeach()

foreach(i RANGE 10 12)
  add_test(NAME phase1_testcase${i}
           COMMAND $<TARGET_FILE:phase1_tester>
                   ${CMAKE_SOURCE_DIR}/test/phase1/testcases/test_1_r${i}.spl)
endforeach()

foreach(i RANGE 1 9)
  add_test(NAME phase1_debug${i}
          COMMAND $<TARGET_FILE:phase1_tester>
          ${CMAKE_SOURCE_DIR}/test/phase1/debug/debug${i}.txt)
endforeach()