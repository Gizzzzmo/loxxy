add_executable(test_string_store test_string_store.cpp)
target_link_libraries(test_string_store GTest::GTest GTest::gtest_main string_store)

add_executable(test_lexer test_lexer.cpp)
target_link_libraries(test_lexer GTest::GTest GTest::gtest_main lexer ast generic_stream)

add_executable(test_tqstream test_tqstream.cpp)
target_link_libraries(test_tqstream GTest::GTest GTest::gtest_main tqstream murmurhash)

add_library(test_variant test_variant.cpp)
if (LOXXY_STD_VARIANT)
    target_compile_definitions(test_variant PRIVATE STD_VARIANT)
else()
    target_link_libraries(test_variant PRIVATE mpark_variant)
endif()

add_test(test_string_store ${CMAKE_CURRENT_BINARY_DIR}/test_string_store)
add_test(test_lexer ${CMAKE_CURRENT_BINARY_DIR}/test_lexer)