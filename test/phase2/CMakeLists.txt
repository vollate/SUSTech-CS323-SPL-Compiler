add_executable(splc_2 "Phase2Tester.cpp")
target_link_libraries(splc_2 PRIVATE spl_frontage)

foreach (i RANGE 1 9)
    add_test(NAME phase2_testcase${i} COMMAND
            $<TARGET_FILE:splc_2>
            ${CMAKE_SOURCE_DIR}/test/phase2/testcases/test_2_r0${i}.spl
            ${CMAKE_SOURCE_DIR}/test/phase2/testout/test_2_r0${i}.out)
endforeach ()

foreach (i RANGE 10 12)
    add_test(NAME phase2_testcase${i} COMMAND
            $<TARGET_FILE:splc_2> ${CMAKE_SOURCE_DIR}/test/phase2/testcases/test_2_r${i}.spl
            ${CMAKE_SOURCE_DIR}/test/phase2/testout/test_2_r${i}.out)
endforeach ()

#foreach (i RANGE 1 9)
#    add_test(NAME phase2_additional_testcase${i} COMMAND
#            $<TARGET_FILE:splc_2>
#            ${CMAKE_SOURCE_DIR}/test/phase2/extra-test/test_1_s0${i}.spl
#            ${CMAKE_SOURCE_DIR}/test/phase2/testout/test_1_s0${i}.out)
#endforeach ()
#
#foreach (i RANGE 10 13)
#    add_test(NAME phase2_additional_testcase${i} COMMAND
#            $<TARGET_FILE:splc_2> ${CMAKE_SOURCE_DIR}/test/phase2/extra-test/test_1_s${i}.spl
#            ${CMAKE_SOURCE_DIR}/test/phase2/testout/test_1_s${i}.out)
#endforeach ()
#foreach (i RANGE 1 1)
#    add_test(
#            NAME phase2_debug${i}
#            COMMAND
#            $<TARGET_FILE:splc_2> ${CMAKE_SOURCE_DIR}/test/phase2/debug/debug${i}.txt
#            ${CMAKE_SOURCE_DIR}/test/phase2/debugout/debug${i}.out)
#endforeach ()

