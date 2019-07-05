cmake_minimum_required(VERSION 3.10)

project(massivelogger LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

include_directories(include)

add_executable(base_test     tests/base_test.c    )
add_executable(timeline_test tests/timeline_test.c)
add_executable(realloc_test  tests/realloc_test.c )
add_executable(type_test     tests/type_test.c    )

enable_testing()

add_test(NAME base_test     COMMAND $<TARGET_FILE:base_test>    )
add_test(NAME timeline_test COMMAND $<TARGET_FILE:timeline_test>)
add_test(NAME realloc_test  COMMAND $<TARGET_FILE:realloc_test> )
add_test(NAME type_test     COMMAND $<TARGET_FILE:type_test>    )

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)

install(DIRECTORY include/ DESTINATION include)
