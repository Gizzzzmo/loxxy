#include <cstring>
#include <fstream>
#include <iostream>
#include <thread>
#include <utility>
#include <vector>

import utils.tqstream;
import utils.stupid_type_traits;
import parser.rd;
import lexer;
import ast;
import ast.boxed_node_builder;
import ast.printer;

using namespace loxxy;
using namespace utils;

auto main(int argc, const char** argv) -> int {

    bool repl_mode = false;
    const char* filename = nullptr;

    for (int i = 1; i < argc; i++) {
        if (std::strcmp("--repl", argv[i]) == 0) {
            repl_mode = true;
            continue;
        }
        if (filename != nullptr) {
            std::cout << "Usage: parser_repl [--repl] [filename]" << std::endl;
            return 1;
        }
        std::cout << i << std::endl;

        filename = argv[i];
    }

    std::ifstream file;

    if (filename == nullptr) {
        file.open("/dev/stdin");
    } else {
        file.open(filename);
        if (file.fail()) {
            std::cout << "File not found:\n" << filename << std::endl;
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
    std::thread parse_thread([repl_mode, &parser]() {
        while (true) {
            auto root = repl_mode ? parser.parseRepl() : parser.parse();
            if (root.statements.size() == 0)
                break;
            for (auto& stmt : root.statements) {
                std::cout << stmt << "\n";
            }
        }
    });
    lex_thread.join();
    parse_thread.join();

    return 0;
}
