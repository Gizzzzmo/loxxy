function(add_cxx_module target_name)
    add_library(${target_name}) 
    target_sources(${target_name}
        PUBLIC
            FILE_SET CXX_MODULES FILES
                ${ARGN}
    )
endfunction()

add_cxx_module(string_store utils/string_store.cpp)

add_cxx_module(stupid_type_traits
    utils/stupid_type_traits.cpp
)

add_cxx_module(generic_stream
    utils/generic_stream.cpp
)

add_cxx_module(variant
    utils/variant.cpp
)
target_link_libraries(variant PRIVATE stupid_type_traits)

if(LOXXY_STD_VARIANT)
    target_compile_definitions(variant PRIVATE STD_VARIANT)
else()
    target_link_libraries(variant PRIVATE mpark_variant)
endif()


add_cxx_module(murmurhash
    utils/murmurhash/murmurhash.cpp
    utils/murmurhash/murmurhash2.cpp
    #utils/murmurhash/murmurhash3.cpp
)

add_cxx_module(tqstream
    utils/tqstream.cpp
)
target_link_libraries(tqstream PRIVATE SPSCQueue::SPSCQueue)

add_cxx_module(ast
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

add_cxx_module(ast_copier
    ast/visitors/copier.cpp
)
target_link_libraries(ast_copier PRIVATE stupid_type_traits ast variant)

add_cxx_module(ast_extractor
    ast/visitors/extractor.cpp
)
target_link_libraries(ast_extractor PRIVATE stupid_type_traits ast variant)

add_cxx_module(ast_hasher
    ast/visitors/hasher.cpp
)
target_link_libraries(ast_hasher PRIVATE stupid_type_traits ast variant)

add_cxx_module(ast_interpreter
    ast/visitors/interpreter.cpp
)
target_link_libraries(ast_interpreter PRIVATE tsl::hat_trie stupid_type_traits ast variant)

add_cxx_module(ast_printer
    ast/visitors/printer.cpp
)
target_link_libraries(ast_printer PRIVATE stupid_type_traits ast variant)

add_cxx_module(ast_boxed_node_builder
    ast/builders/boxed.cpp
)
target_link_libraries(ast_boxed_node_builder
    PRIVATE
        stupid_type_traits
        ast
        ast_interpreter
        variant
)

add_cxx_module(ast_hash_payload_builder
    ast/builders/hash_payload.cpp
)
target_link_libraries(ast_hash_payload_builder
    PRIVATE
        stupid_type_traits
        ast 
        ast_extractor
)

add_cxx_module(ast_rc_node_builder
    ast/builders/rc.cpp
)
target_link_libraries(ast_rc_node_builder PRIVATE stupid_type_traits ast)

add_cxx_module(ast_offset_builder
    ast/builders/offset.cpp
)
target_link_libraries(ast_offset_builder PRIVATE stupid_type_traits ast)

add_cxx_module(lexer
    lexer.cpp
)
target_link_libraries(lexer PRIVATE string_store ast tsl::hat_trie)

add_cxx_module(parser
    parser/parser.cpp
)
add_cxx_module(rd_parser parser/rd.cpp)
target_link_libraries(rd_parser PRIVATE parser ast_printer ast stupid_type_traits ast_boxed_node_builder)

add_cxx_module(lr_parser
    parser/lr.cpp
)

target_link_libraries(lr_parser PRIVATE ast_printer ast stupid_type_traits ast_boxed_node_builder)
