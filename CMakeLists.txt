cmake_minimum_required(VERSION 3.29.0)

project(loxxy LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(
    LOXXY_STD_VARIANT
    "Use variant from the standard library as opposed to mpark's variant"
    ON
)

# external dependencies
find_package(GTest)
find_package(LLVM)
if(NOT LOXXY_STD_VARIANT)
    add_subdirectory(variant)
endif()
add_subdirectory(SPSCQueue)
add_subdirectory(hat-trie)
add_subdirectory(perf-cpp)
add_library(perfcpp INTERFACE)
target_link_libraries(perfcpp INTERFACE perf-cpp)
target_include_directories(perfcpp INTERFACE ./perf-cpp/include/)

add_subdirectory(lib)
add_subdirectory(bin)

option(LOXXY_TESTING "Enable Testing" ON)

if(LOXXY_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()
