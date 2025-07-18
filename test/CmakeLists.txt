add_executable(movehex_test movehex_test.cpp)

add_test(NAME edge_cases COMMAND movehex_test ${CMAKE_HOME_DIRECTORY}/test/edge_cases.txt ${CMAKE_HOME_DIRECTORY}/test/edge_cases.txt.result)
add_test(NAME empty COMMAND movehex_test ${CMAKE_HOME_DIRECTORY}/test/empty.txt ${CMAKE_HOME_DIRECTORY}/test/empty.txt.result)
add_test(NAME example COMMAND movehex_test ${CMAKE_HOME_DIRECTORY}/test/example.txt ${CMAKE_HOME_DIRECTORY}/test/example.txt.result)
add_test(NAME large COMMAND movehex_test ${CMAKE_HOME_DIRECTORY}/test/large.txt ${CMAKE_HOME_DIRECTORY}/test/large.txt.result)
add_test(NAME long COMMAND movehex_test ${CMAKE_HOME_DIRECTORY}/test/long.txt ${CMAKE_HOME_DIRECTORY}/test/long.txt.result)
add_test(NAME stress_cache COMMAND movehex_test ${CMAKE_HOME_DIRECTORY}/test/stress_cache.txt ${CMAKE_HOME_DIRECTORY}/test/stress_cache.txt.result)
add_test(NAME stress_cache2 COMMAND movehex_test ${CMAKE_HOME_DIRECTORY}/test/stress_cache2.txt ${CMAKE_HOME_DIRECTORY}/test/stress_cache2.txt.result)

add_custom_command(TARGET movehex_test POST_BUILD
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running tests..."
)