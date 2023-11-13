add_executable(splc "Phase1Tester.cpp")
target_link_libraries(splc PRIVATE spl_frontage)

foreach (i RANGE 1 9)
    add_test(NAME phase1_testcase${i} COMMAND
            $<TARGET_FILE:splc>
            ${CMAKE_SOURCE_DIR}/test/phase1/testcases/test_1_r0${i}.spl
            ${CMAKE_SOURCE_DIR}/test/phase1/testout/test_1_r0${i}.out)
endforeach ()

foreach (i RANGE 10 12)
    add_test(NAME phase1_testcase${i} COMMAND
            $<TARGET_FILE:splc> ${CMAKE_SOURCE_DIR}/test/phase1/testcases/test_1_r${i}.spl
            ${CMAKE_SOURCE_DIR}/test/phase1/testout/test_1_r${i}.out)
endforeach ()

foreach (i RANGE 1 9)
    add_test(NAME phase1_additional_testcase${i} COMMAND
            $<TARGET_FILE:splc>
            ${CMAKE_SOURCE_DIR}/test/phase1/extra-test/test_1_s0${i}.spl
            ${CMAKE_SOURCE_DIR}/test/phase1/testout/test_1_s0${i}.out)
endforeach ()

foreach (i RANGE 10 13)
    add_test(NAME phase1_additional_testcase${i} COMMAND
            $<TARGET_FILE:splc> ${CMAKE_SOURCE_DIR}/test/phase1/extra-test/test_1_s${i}.spl
            ${CMAKE_SOURCE_DIR}/test/phase1/testout/test_1_s${i}.out)
endforeach ()
foreach (i RANGE 1 1)
    add_test(
            NAME phase1_debug${i}
            COMMAND
            $<TARGET_FILE:splc> ${CMAKE_SOURCE_DIR}/test/phase1/debug/debug${i}.txt
            ${CMAKE_SOURCE_DIR}/test/phase1/debugout/debug${i}.out)
endforeach ()

