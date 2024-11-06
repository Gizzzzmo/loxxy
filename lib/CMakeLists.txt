


add_library(string_store)
target_sources(string_store
    PUBLIC
        FILE_SET CXX_MODULES FILES
            utils/string_store.cpp
)


add_library(stupid_type_traits)
target_sources(stupid_type_traits
    PUBLIC
        FILE_SET CXX_MODULES FILES
            utils/stupid_type_traits.cpp
)

add_library(variant)
target_sources(variant
    PUBLIC
        FILE_SET CXX_MODULES FILES
            utils/variant.cpp
)
target_link_libraries(variant PRIVATE stupid_type_traits)

if(LOXXY_STD_VARIANT)
    target_compile_definitions(variant PRIVATE STD_VARIANT)
else()
    target_link_libraries(variant PRIVATE mpark_variant)
endif()


add_library(murmurhash)
target_sources(murmurhash
    PUBLIC
        FILE_SET CXX_MODULES FILES
            utils/murmurhash/murmurhash.cpp
            utils/murmurhash/murmurhash2.cpp
            #utils/murmurhash/murmurhash3.cpp
)

add_library(tqstream)
target_sources(tqstream
    PUBLIC
        FILE_SET CXX_MODULES FILES
            utils/tqstream.cpp
)
target_link_libraries(tqstream PRIVATE SPSCQueue::SPSCQueue)

add_library(ast)
target_sources(ast
    PUBLIC
        FILE_SET CXX_MODULES FILES
            ast/ast.cpp
            ast/nodes.cpp
            ast/value.cpp
            ast/building.cpp
            ast/token.cpp
            ast/hash.cpp
            ast/type_traits.cpp
)
target_link_libraries(ast
    PRIVATE
        stupid_type_traits
        string_store
        murmurhash
        variant
)
target_include_directories(ast PUBLIC ast/include)

add_library(ast_copier)
target_sources(ast_copier
    PUBLIC
        FILE_SET CXX_MODULES FILES
            ast/visitors/copier.cpp
)
target_link_libraries(ast_copier PRIVATE stupid_type_traits ast variant)

add_library(ast_extractor)
target_sources(ast_extractor
    PUBLIC
        FILE_SET CXX_MODULES FILES
            ast/visitors/extractor.cpp
)
target_link_libraries(ast_extractor PRIVATE stupid_type_traits ast variant)

add_library(ast_hasher)
target_sources(ast_hasher
    PUBLIC
        FILE_SET CXX_MODULES FILES
            ast/visitors/hasher.cpp
)
target_link_libraries(ast_hasher PRIVATE stupid_type_traits ast variant)

add_library(ast_interpreter)
target_sources(ast_interpreter
    PUBLIC
        FILE_SET CXX_MODULES FILES
            ast/visitors/interpreter.cpp
)
target_link_libraries(ast_interpreter PRIVATE stupid_type_traits ast variant)

add_library(ast_printer)
target_sources(ast_printer
    PUBLIC
        FILE_SET CXX_MODULES FILES
            ast/visitors/printer.cpp
)
target_link_libraries(ast_printer PRIVATE stupid_type_traits ast variant)

add_library(ast_boxed_node_builder)
target_sources(ast_boxed_node_builder
    PUBLIC
        FILE_SET CXX_MODULES FILES
            ast/builders/boxed.cpp
)
target_link_libraries(ast_boxed_node_builder PRIVATE stupid_type_traits ast ast_interpreter variant)

add_library(ast_hash_payload_builder)
target_sources(ast_hash_payload_builder
    PUBLIC
        FILE_SET CXX_MODULES FILES
            ast/builders/hash_payload.cpp
)
target_link_libraries(ast_hash_payload_builder PRIVATE stupid_type_traits ast ast_extractor)

add_library(ast_rc_node_builder)
target_sources(ast_rc_node_builder
    PUBLIC
        FILE_SET CXX_MODULES FILES
            ast/builders/rc.cpp
)
target_link_libraries(ast_rc_node_builder PRIVATE stupid_type_traits ast)

add_library(ast_offset_builder)
target_sources(ast_offset_builder
    PUBLIC
        FILE_SET CXX_MODULES FILES
            ast/builders/offset.cpp
)
target_link_libraries(ast_offset_builder PRIVATE stupid_type_traits ast)

add_library(lexer)
target_sources(lexer
    PUBLIC
        FILE_SET CXX_MODULES FILES
            lexer.cpp
)
target_link_libraries(lexer PRIVATE string_store ast tsl::hat_trie)

add_library(parser)
target_sources(parser
    PUBLIC
        FILE_SET CXX_MODULES FILES
            parser/parser.cpp
            parser/left_recursive.cpp
)
target_link_libraries(parser PRIVATE ast_printer ast stupid_type_traits ast_boxed_node_builder)
