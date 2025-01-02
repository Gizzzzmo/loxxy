import lexer;
import ast;
#include <fstream>
#include <iostream>

using namespace loxxy;

struct printing_stream {
    template<typename... Args>
    void emplace(Args&&... args) {
        std::cout << Token(std::forward<Args>(args)...) << std::endl;
    }
};

auto main(int argc, const char** argv) -> int {
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

    Loxxer lexer(std::move(file), printing_stream{});

    lexer.scanTokens();
    return 0;
}
