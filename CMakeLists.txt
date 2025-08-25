cmake_minimum_required(VERSION 3.10)
project(json-parser VERSION 1.5.3)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(CMAKE_C_FLAGS_RELEASE "-O2")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

message(STATUS "Using C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "Compiler path: ${CMAKE_C_COMPILER_ID}")

add_library(json-parser STATIC
    json_parser.c
)

add_executable(parse_json
    test.c
)

target_link_libraries(parse_json json-parser)

add_executable(test_speed
    test_speed.c
)

target_link_libraries(test_speed json-parser)
