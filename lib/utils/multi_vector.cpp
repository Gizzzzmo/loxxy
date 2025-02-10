module;
#include <tuple>
#include <vector>
export module utils.multi_vector;

import utils.stupid_type_traits;

export namespace utils {

template <typename... Ts>
struct multi_vector {
    template <typename Self, typename T, typename offset_t>
    auto operator[](this Self&& self, offset_pointer<T, offset_t>& ptr) -> decltype(auto) {
        return std::get<std::vector<T>>(self.vectors)[ptr.offset];
    }
    template <typename T, typename Self>
    auto get_vec(this Self&& self) -> decltype(auto) {
        return std::get<std::vector<T>>(self.vectors);
    }

private:
    std::tuple<std::vector<Ts>...> vectors;
};

} // namespace utils
