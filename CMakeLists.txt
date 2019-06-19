cmake_minimum_required(VERSION 3.10)

project(massivelogger LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

include_directories(include)

add_executable(mlog_test tests/mlog_test.c)

enable_testing()

add_test(
  NAME    mlog_test
  COMMAND $<TARGET_FILE:mlog_test>
)

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)

install(DIRECTORY include/ DESTINATION include)

