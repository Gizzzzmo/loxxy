module;
#include <cstddef>
#include <span>
#include <string>
#include <vector>
export module ast:value;
import utils.variant;
import utils.string_store;

export namespace loxxy {
struct BuiltinCallable;
struct LoxCallable;

using Value = utils::variant<bool, double, std::string, std::nullptr_t, const BuiltinCallable*, LoxCallable>;

struct BuiltinCallable {
    Value (*fn)(std::span<Value>);
    const char* name;
    size_t arity;
};

struct LoxCallable {
    std::vector<const utils::persistent_string<>*> arg_names;
    const void* function_body;
};

} // namespace loxxy
