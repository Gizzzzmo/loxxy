module;
#include <cstddef>
#include <string>
export module ast:value;
import utils.variant;

export namespace loxxy {

using Value = utils::variant<bool, double, std::string, std::nullptr_t>;

} // namespace loxxy (exported)