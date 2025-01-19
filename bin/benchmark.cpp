#include <chrono>
#include <cmath>
#include <cstdint>
#include <deque>
#include <fstream>
#include <ios>
#include <iostream>
#include <numeric>
#include <thread>
#include <utility>
#include <vector>

import utils.tqstream;
import utils.stupid_type_traits;
import utils.generic_stream;
import parser.rd;
import lexer;
import ast;
import ast.boxed_node_builder;
import ast.rc_node_builder;
import ast.offset_builder;
import ast.hash_payload_builder;

using namespace loxxy;
using namespace utils;

#ifdef __GNUG__
#include <cstdlib>
#include <cxxabi.h>
#include <memory>

auto demangle(const char* name) -> std::string {

    int status = -4; // some arbitrary value to eliminate the compiler warning

    // enable c++11 by passing the flag -std=c++11 to g++
    std::unique_ptr<char, void (*)(void*)> res{abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free};

    return (status == 0) ? res.get() : name;
}

#else

// does nothing if not g++
std::string demangle(const char* name) { return name; }

#endif

template <typename... Ts>
void for_types(auto&& f) {
    (f.template operator()<Ts>(), ...);
}

using std::chrono::duration;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;
using std::chrono::milliseconds;

auto main(int argc, const char** argv) -> int {
    std::ifstream file;
    if (argc < 2) {
        std::cerr << "Need a file to read: benchmark <file>" << std::endl;
        return 1;
    } else {
        file.open(argv[1]);
        if (file.fail()) {
            std::cout << "File not found:\n" << argv[1] << std::endl;
            return 1;
        }
    }

    generic_stream<std::vector, Token> token_stream;
    generic_stream<std::vector, char> char_stream;

    int mult = 1;
    if (argc > 2)
        mult = std::stoi(argv[2]);

    size_t n_chars = 0;
    while (!file.eof()) {
        char_stream.putback(file.get());
        n_chars++;
    }
    char_stream.v.reserve((mult - 1) * n_chars);
    for (int i = 0; i < mult; i++) {
        for (size_t j = 0; j < n_chars; j++) {
            char_stream.putback(char_stream.v[j]);
        }
    }
    file.close();
    n_chars *= mult;
    std::cout << "num of chars: " << n_chars << "\n";

    Loxxer lexer(char_stream, token_stream);
    std::vector<double> times;
    for (int i = 0; i < 5; i++) {
        token_stream.v.clear();
        token_stream.v.shrink_to_fit();
        auto t1 = high_resolution_clock::now();
        lexer.scanTokens();
        auto t2 = high_resolution_clock::now();
        duration<double, std::milli> ms_double = t2 - t1;
        times.push_back(ms_double.count());
        std::cout << "  " << times.back() << std::endl;
        char_stream.reset();
    }
    double mean = std::accumulate(times.begin(), times.end(), 0.0, std::plus<>()) / times.size();
    std::cout << "mean:   " << mean << std::endl;
    auto variance_func = [&mean, &times](double accumulator, const double& val) {
        return accumulator + ((val - mean) * (val - mean) / (times.size() - 1));
    };
    double variance = std::accumulate(times.begin(), times.end(), 0.0, variance_func);

    double stddev = std::sqrt(variance);
    std::cout << "stddev: " << stddev << std::endl;

    // Parser<
    //     generic_stream<std::vector, Token>,
    //     RCNodeBuilder<NodeHash, true, HashPayloadBuilder<SharedPtrIndirection, true>>
    //>
    using BoxParse = Parser<generic_stream<std::vector, Token>, BoxedNodeBuilder<>>;

    using OffsetParse = Parser<generic_stream<std::vector, Token>, OffsetBuilder<uint32_t>>;
    std::cout << "Parsers:\n";
    for_types<BoxParse, OffsetParse>([&token_stream]<typename T>() {
        std::cout << demangle(typeid(T).name()) << "\n";
        std::vector<double> times;
        for (int i = 0; i < 5; i++) {
            T parser(token_stream);
            auto t1 = high_resolution_clock::now();
            parser.parse();
            auto t2 = high_resolution_clock::now();
            duration<double, std::milli> ms_double = t2 - t1;
            times.push_back(ms_double.count());
            token_stream.reset();
        }
        double mean =
            std::accumulate(times.begin(), times.end(), 0.0, std::plus<>()) / static_cast<double>(times.size());
        std::cout << "mean:   " << mean << "\n";
        auto variance_func = [&mean, &times](double accumulator, const double& val) {
            return accumulator + ((val - mean) * (val - mean) / (times.size() - 1));
        };
        double variance = std::accumulate(times.begin(), times.end(), 0.0, variance_func);

        double stddev = std::sqrt(variance);
        std::cout << "stddev: " << stddev << "\n";
    });

    size_t buf_size = 1;
    if (argc > 3)
        buf_size = std::stoull(argv[3]);

    tqstream<Token> token_stream_p(1024 * 16, buf_size);

    Loxxer lexer_p(char_stream, token_stream_p);

    Parser parser_p(token_stream_p, BoxedNodeBuilder<>{});

    std::vector<double> times_p;
    for (int i = 0; i < 5; i++) {
        duration<double, std::milli> lex_t;
        duration<double, std::milli> parse_t;
        auto t1 = high_resolution_clock::now();

        std::thread lex_thread([&lexer_p, &lex_t, &token_stream_p]() {
            auto t1 = high_resolution_clock::now();
            lexer_p.scanTokens();
            token_stream_p.flush();
            auto t2 = high_resolution_clock::now();
            lex_t = t2 - t1;
        });

        std::thread parse_thread([&parser_p, &parse_t]() {
            auto t1 = high_resolution_clock::now();
            parser_p.parse();
            auto t2 = high_resolution_clock::now();
            parse_t = t2 - t1;
        });

        lex_thread.join();
        parse_thread.join();
        auto t2 = high_resolution_clock::now();
        duration<double, std::milli> ms_double = t2 - t1;
        times_p.push_back(ms_double.count());
        std::cout << "  " << times_p.back() << std::endl;
        std::cout << "    lex: " << lex_t.count() << std::endl;
        std::cout << "    parse: " << parse_t.count() << std::endl;
        char_stream.reset();
        parser_p.reset();
    }

    mean = std::accumulate(times_p.begin(), times_p.end(), 0.0, std::plus<>()) / static_cast<double>(times_p.size());
    std::cout << "mean:   " << mean << "\n";
    auto variance_func_p = [&mean, &times_p](double accumulator, const double& val) {
        return accumulator + ((val - mean) * (val - mean) / (times_p.size() - 1));
    };
    variance = std::accumulate(times_p.begin(), times_p.end(), 0.0, variance_func_p);

    stddev = std::sqrt(variance);
    std::cout << "stddev: " << stddev << "\n";

    return 0;
}
