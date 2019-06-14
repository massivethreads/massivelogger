cmake_minimum_required(VERSION 3.10)

project(massivelogger LANGUAGES C)

set(CMAKE_C_STANDARD 99)

include_directories(include)

add_executable(mlog_test tests/mlog_test.c)

enable_testing()

add_test(
  NAME    mlog_test
  COMMAND $<TARGET_FILE:mlog_test>
)
