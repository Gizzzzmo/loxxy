#include <chrono>
#include <fstream>
#include <iostream>
#include <span>
#include <thread>

import utils.tqstream;
import utils.stupid_type_traits;
import utils.string_store;
import utils.variant;

import parser.rd;
import lexer;
import ast;
import ast.boxed_node_builder;
import ast.printer;
import ast.llvm;

using namespace loxxy;
using namespace utils;

auto main(int argc, const char** argv) -> int {
    std::ifstream file;
    bool (*flushCondition)(const Token& t);

    if (argc < 2) {
        file.open("/dev/stdin");
        flushCondition = [](const Token& t) { return t.getType() == NEW_LINE; };
    } else {
        file.open(argv[1]);
        if (file.fail()) {
            std::cout << "File not found:\n" << argv[1] << std::endl;
            return 1;
        }
        flushCondition = [](const Token& t) { return false; };
    }

    tqstream<Token> token_stream(512, 64, flushCondition);

    Loxxer lexer(std::move(file), token_stream);

    Parser parser(token_stream, BoxedNodeBuilder<>{});

    std::thread lex_thread([&lexer, &token_stream, argc]() {
        if (argc < 2)
            lexer.scanTokensLine();
        else
            lexer.scanTokens();

        token_stream.flush();
    });

    std::thread parse_thread([&parser, argc]() {
        IRGenerator<empty, UniquePtrIndirection, true> ir_gen;

        while (true) {
            auto root = argc < 2 ? parser.parseRepl() : parser.parse();
            if (root.statements.size() == 0)
                break;
            for (auto& stmt : root.statements) {
                utils::visit(ir_gen, stmt);
            }
        }

        ir_gen.printIR();
    });
    lex_thread.join();
    parse_thread.join();

    return 0;
}
