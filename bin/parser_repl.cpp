#include <fstream>
#include <thread>
#include <iostream>

import utils.tqstream;
import utils.stupid_type_traits;
import parser;
import lexer;
import ast;
import ast.boxed_node_builder;
import ast.rc_node_builder;
import ast.offset_builder;
import ast.hash_payload_builder;

using namespace loxxy;
using namespace utils;


int main (int argc, const char** argv) {
    std::ifstream file;
    if (argc < 2) {
        file.open("/dev/stdin");
    } 
    else {
        file.open(argv[1]);
        if (file.fail()) {
            std::cout << "File not found:\n" << argv[1] << std::endl; 
            return 1;
        }
    }

    tqstream<Token> token_stream(64);
    tqstream<Token> token_stream2(64);

    Loxxer lexer(std::move(file), [&token_stream](Token&& token) {
        token_stream.putback(std::move(token));
        //token_stream2.putback(std::move(token));
        return true;
    });

    //Parser<
    //    tqstream<Token>,
    //    NodeHash,
    //    SharedPtrIndirection,
    //    true,
    //    RCNodeBuilder<NodeHash, true, HashPayloadBuilder<SharedPtrIndirection, true>>
    //> parser(token_stream);

    Parser<
        tqstream<Token>,
        NodeHash,
        UniquePtrIndirection,
        true,
        BoxedNodeBuilder<NodeHash, true, HashPayloadBuilder<UniquePtrIndirection, true>>
    > parser(token_stream);

    Parser<
        tqstream<Token>,
        NodeHash,
        OffsetPointerIndirection<uint32_t>,
        true,
        OffsetBuilder<uint32_t, NodeHash>
    > parser2(token_stream2, OffsetBuilder<uint32_t, NodeHash>{});
    
    std::thread lex_thread([&lexer]() {
        lexer.scanTokens();
    });
    std::thread parse_thread([&parser](){
        parser.parse();
    });
    std::thread parse_thread2([]() {
        //parser2.parse();
    });

    lex_thread.join();
    parse_thread.join();
    parse_thread2.join();


    return 0;
}
