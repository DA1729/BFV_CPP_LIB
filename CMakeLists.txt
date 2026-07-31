cmake_minimum_required(VERSION 3.16)
project(bfv_cpp VERSION 0.2.0 LANGUAGES CXX)

option(BFV_BUILD_TESTS "Build the test suite" ON)
option(BFV_BUILD_EXAMPLES "Build the examples" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

add_library(bfv
    src/mod_arith.cpp
    src/rng.cpp
    src/prime.cpp
    src/ntt.cpp
    src/params.cpp
    src/poly.cpp
    src/context.cpp
    src/keys.cpp
    src/encoder.cpp
    src/encryptor.cpp
    src/decryptor.cpp
    src/evaluator.cpp
)
add_library(bfv::bfv ALIAS bfv)

target_include_directories(bfv PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(bfv PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BFV_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(BFV_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

include(GNUInstallDirs)
install(TARGETS bfv EXPORT bfv-targets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/bfv DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT bfv-targets NAMESPACE bfv:: DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bfv)
