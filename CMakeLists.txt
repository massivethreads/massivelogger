cmake_minimum_required(VERSION 3.10)

project(massivelogger LANGUAGES C CXX)

file(GLOB test_files tests/*_test.c)

if(COMPILE_AS_CPP)
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
  set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

  set_source_files_properties(${test_files} PROPERTIES LANGUAGE CXX)
else()
  set(CMAKE_C_STANDARD 99)
  set(CMAKE_C_STANDARD_REQUIRED ON)
  set(CMAKE_C_EXTENSIONS OFF)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
endif()

include_directories(include)
enable_testing()

foreach(file_path ${test_files})
  get_filename_component(test_name ${file_path} NAME_WE)
  add_executable(${test_name} ${file_path})
  add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}>)
endforeach()

add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure)

install(DIRECTORY include/ DESTINATION include)
