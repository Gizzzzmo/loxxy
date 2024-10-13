add_executable(test_string_store test_string_store.cpp)
target_link_libraries(test_string_store GTest::GTest GTest::gtest_main string_store)

add_executable(test_lexer test_lexer.cpp)
target_link_libraries(test_lexer GTest::GTest GTest::gtest_main lexer ast)

add_library(test_variant test_variant.cpp)
if (LOXXY_STD_VARIANT)
    target_compile_definitions(test_variant PRIVATE STD_VARIANT)
else()
    target_link_libraries(test_variant PRIVATE mpark_variant)
endif()

add_test(test_string_store ${CMAKE_CURRENT_BINARY_DIR}/test_string_store)
add_test(test_lexer ${CMAKE_CURRENT_BINARY_DIR}/test_lexer)