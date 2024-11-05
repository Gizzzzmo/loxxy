add_executable(lexer_repl lexer_repl.cpp)
add_executable(parser_repl parser_repl.cpp)
add_executable(playground test.cpp)

target_link_libraries(lexer_repl PRIVATE lexer)
target_link_libraries(parser_repl
    PRIVATE
        tqstream
        lexer
        parser
        ast
        ast_boxed_node_builder
        ast_rc_node_builder
        ast_offset_builder
        ast_hash_payload_builder
        ast_interpreter
)
target_link_libraries(playground PRIVATE variant)
