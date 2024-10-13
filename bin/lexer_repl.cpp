import lexer;
import ast;
#include <fstream>
#include <iostream>

using namespace loxxy;


int main(int argc, const char** argv) {
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

    Loxxer lexer(std::move(file), [](Token&& token) {
        std::cout << token << std::endl;
        return true;
    });

    lexer.scanTokens();
    return 0;
}
