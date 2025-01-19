module;
#include <utility>
export module parser.lr;

import ast;
import ast.boxed_node_builder;

template <typename Stream, typename Builder>
class LRParser {
public:
    template <typename... Args>
    LRParser(Stream& stream, Args&&... args) : stream(stream), builder(std::forward<Args>(args)...) {}

private:
    Stream& stream;
    Builder builder;
};
