cmake_minimum_required(VERSION 3.14)
project(lz-compressor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Library
add_library(lzlib
    src/lz77.cpp
    src/huffman.cpp
    src/deflate.cpp
    src/stream.cpp
)
target_include_directories(lzlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# CLI tool
add_executable(lz-compressor src/main.cpp)
target_link_libraries(lz-compressor PRIVATE lzlib)

# Tests
enable_testing()

# Fetch Catch2 header-only
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

add_executable(tests
    test/test_lz77.cpp
    test/test_huffman.cpp
    test/test_deflate.cpp
    test/test_stream.cpp
)
target_link_libraries(tests PRIVATE lzlib Catch2::Catch2WithMain)

include(CTest)
include(Catch2/extras/Catch.cmake OPTIONAL)
add_test(NAME unit_tests COMMAND tests)
