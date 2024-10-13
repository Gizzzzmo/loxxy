#include <functional>
#include <memory>
#include <type_traits>
#include <variant>
#include <cassert>

import utils.stupid_type_traits;
import ast;
import ast.interpreter;

using namespace loxxy;

int main() {
    BoxedSTN<> node = std::make_unique<NumberNode<>>(
        empty{}, 9
    );


    Interpreter<empty, UniquePtrIndirection, true, void> interpreter;
    Value x1 = interpreter(NumberNode<>{{}, 9});
    Value x2 = interpreter(NilNode<>{{}});
    Value x3 = interpreter(BoolNode<>{{}, true});

    Value x4 = interpreter(GroupingNode<>{{}, std::move(node)});
    //visit(interpreter, );

    return 0;
}
