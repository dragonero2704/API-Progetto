add_executable(movhex_test movhex_test.cpp)

target_compile_features(movhex_test PRIVATE cxx_std_17)

set(EXECUTABLE_PATH ${CMAKE_HOME_DIRECTORY}/build/movhex)

if(WIN32)
    set(EXECUTABLE_PATH ${EXECUTABLE_PATH}.exe)
endif()

add_test(NAME edge_cases COMMAND movhex_test ${EXECUTABLE_PATH} ${CMAKE_HOME_DIRECTORY}/test/edge_cases.txt ${CMAKE_HOME_DIRECTORY}/test/edge_cases.txt.result)
add_test(NAME empty COMMAND movhex_test ${EXECUTABLE_PATH} ${CMAKE_HOME_DIRECTORY}/test/empty.txt ${CMAKE_HOME_DIRECTORY}/test/empty.txt.result)
add_test(NAME example COMMAND movhex_test ${EXECUTABLE_PATH} ${CMAKE_HOME_DIRECTORY}/test/example.txt ${CMAKE_HOME_DIRECTORY}/test/example.txt.result)
add_test(NAME large COMMAND movhex_test ${EXECUTABLE_PATH} ${CMAKE_HOME_DIRECTORY}/test/large.txt ${CMAKE_HOME_DIRECTORY}/test/large.txt.result)
add_test(NAME long COMMAND movhex_test ${EXECUTABLE_PATH} ${CMAKE_HOME_DIRECTORY}/test/long.txt ${CMAKE_HOME_DIRECTORY}/test/long.txt.result)
add_test(NAME stress_cache COMMAND movhex_test ${EXECUTABLE_PATH} ${CMAKE_HOME_DIRECTORY}/test/stress_cache.txt ${CMAKE_HOME_DIRECTORY}/test/stress_cache.txt.result)
add_test(NAME stress_cache2 COMMAND movhex_test ${EXECUTABLE_PATH} ${CMAKE_HOME_DIRECTORY}/test/stress_cache2.txt ${CMAKE_HOME_DIRECTORY}/test/stress_cache2.txt.result)