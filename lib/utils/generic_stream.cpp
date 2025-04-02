module;
#include <cassert>
#include <deque>
#include <utility>

export module utils.generic_stream;

export namespace utils {

template <template <typename...> class Container, typename T>
struct generic_stream {
    template <typename... Args>
    generic_stream(Args&&... args) : v(std::forward<Args>(args)...) {}
    auto get() -> T& {
        assert(index < v.size());
        return v[index++];
    }
    auto peek() -> const T& {
        assert(index < v.size());
        return v[index];
    }
    template <typename U>
    void putback(U&& x) {
        v.push_back(std::forward<U>(x));
    }
    template <typename... Args>
    void emplace(Args&&... args) {
        v.emplace_back(std::forward<Args>(args)...);
    }
    void reset() { index = 0; }

    auto eof() -> bool { return v.size() == index; }
    auto fail() -> bool { return false; }
    size_t index = 0;
    Container<T> v;
};

template <typename T>
struct generic_stream<std::deque, T> {
    template <typename... Args>
    generic_stream(Args&&... args) : v(std::forward<Args>(args)...) {}
    auto get() -> T {
        T t = std::move(v.front());
        v.pop_front();
        return t;
    }
    auto peek() -> const T& { return v.front(); }
    template <typename U>
    void putback(U&& x) {
        v.push_back(std::forward<U>(x));
    }
    template <typename... Args>
    void emplace(Args&&... args) {
        v.emplace_back(std::forward<Args>(args)...);
    }
    auto eof() -> bool { return v.empty(); }
    auto fail() -> bool { return false; }
    std::deque<T> v;
};

} // namespace utils
