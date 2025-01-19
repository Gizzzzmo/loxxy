#include <cassert>
#include <iostream>
#include <type_traits>
#include <utility>

import utils.variant;

using namespace utils;

struct Test : WrappedVar<variant<int, double>> {
    using WrappedVar<variant<int, double>>::WrappedVar;
};

struct Visitor {
    void operator()(int) { std::cout << "int" << std::endl; }

    void operator()(double) { std::cout << "double" << std::endl; }
};

int main() {

    Test test{0};

    visit(Visitor{}, test);
    if (holds_alternative<int>(test))
        std::cout << "heyo " << get<int>(test) << "\n";
    if (holds_alternative<double>(test))
        std::cout << "heya " << get<double>(test) << "\n";

    auto blub = get_if<int>(&test);
    auto blab = get_if<double>(&test);

    std::cout << blub << " " << blab << "\n";

    variant<int, double> yeet{0.8};
    yeet = 10;

    // .if (std::holds_alternative<int>(str)) {
    //     std::cout << "hoi" << std::endl;
    // }
    //

    // visit(interpreter, );

    return 0;
}

#if 0 

    BoxedExpr<> node = std::make_unique<NumberExpr<>>(
        empty{}, 9
    );
Interpreter<empty, UniquePtrIndirection, true, void> interpreter;
    Value x1 = interpreter(NumberExpr<>{{}, 9});
    Value x2 = interpreter(NilExpr<>{{}});
    Value x3 = interpreter(BoolExpr<>{{}, true});

    Value x4 = interpreter(GroupingExpr<>{{}, std::move(node)});

#endif