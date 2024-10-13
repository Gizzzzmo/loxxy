cmake_minimum_required(VERSION 3.29.0)

project(loxxy LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(LOXXY_STD_VARIANT "Use variant from the standard library as opposed to mpark's variant" ON)


# external dependencies
find_package(GTest)
if(NOT LOXXY_STD_VARIANT)
    add_subdirectory(variant)
endif()
add_subdirectory(SPSCQueue)
add_subdirectory(hat-trie)

add_subdirectory(lib)
add_subdirectory(bin)

option(LOXXY_TESTING "Enable Testing" ON)

if (LOXXY_TESTING)
    enable_testing()
    add_subdirectory(test)
endif()

