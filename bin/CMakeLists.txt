add_executable(lexer_repl lexer_repl.cpp)
add_executable(parser_repl parser_repl.cpp)
add_executable(lox lox.cpp)
add_executable(loxc loxc.cpp)
add_executable(benchmark benchmark.cpp)

target_link_libraries(lexer_repl PRIVATE lexer)
target_link_libraries(
    parser_repl
    PRIVATE tqstream lexer rd_parser ast ast_boxed_node_builder ast_printer
)
target_link_libraries(
    lox
    PRIVATE tqstream lexer rd_parser ast ast_boxed_node_builder ast_interpreter
)
target_link_libraries(
    loxc
    PRIVATE tqstream lexer rd_parser ast ast_offset_builder ast_llvm
)

target_link_libraries(
    benchmark
    PRIVATE
        tqstream
        lexer
        rd_parser
        ast
        ast_boxed_node_builder
        ast_rc_node_builder
        ast_offset_builder
        ast_offset_dedupl_builder
        ast_hash_payload_builder
        ast_interpreter
        generic_stream
        perfcpp
)
