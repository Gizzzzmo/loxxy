#include <fstream>
#include <thread>
#include <iostream>

import utils.tqstream;
import utils.stupid_type_traits;
import utils.variant;
import parser.rd;
import lexer;
import ast;
import ast.boxed_node_builder;
import ast.printer;
import ast.interpreter;

using namespace loxxy;
using namespace utils;


auto main (int argc, const char** argv) -> int {
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

    tqstream<Token> token_stream(512, 64, [](const Token& t) { return t.getType() == NEW_LINE; });

    Loxxer lexer(std::move(file), token_stream);

    Parser parser(token_stream, BoxedNodeBuilder<>{});
    
    std::thread lex_thread([&lexer, &token_stream]() {
        lexer.scanTokensLine();
        token_stream.flush();
    });
    std::thread parse_thread([&parser](){
        Interpreter<empty, UniquePtrIndirection, true> interpreter{};
        while(true) {
            auto root = parser.parse();
            if (root.statements.size() == 0)
                break;
            for (auto& stmt : root.statements) {
                utils::visit(interpreter, stmt);
            }
        }
    });
    lex_thread.join();
    parse_thread.join();


    return 0;
}

