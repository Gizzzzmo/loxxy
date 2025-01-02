module;
#include <utility>
#include <deque>
#include <cassert>

export module utils.generic_stream;

export namespace utils {


template<template<typename...> class Container, typename T>
struct generic_stream {
    template<typename... Args>
    generic_stream(Args&&... args) : v(std::forward<Args>(args)...) {}
    T& get() {
        assert(index < v.size());
        return v[index++];
    }
    const T& peek() {
        assert(index < v.size());
        return v[index];
    }
    template<typename U>
    void putback(U&& x) {
        v.push_back(std::forward<U>(x));
    }
    template<typename... Args>
    void emplace(Args&&... args) {
        v.emplace_back(std::forward<Args>(args)...);
    }
    void reset() {
        index = 0;
    }

    bool eof() {
        return v.size() == index;
    }
    bool fail() {
        return false;
    }
    size_t index = 0;
    Container<T> v;
};

template<typename T>
struct generic_stream<std::deque, T>{
    template<typename... Args>
    generic_stream(Args&&... args) : v(std::forward<Args>(args)...) {}
    T get() {
        T t = std::move(v.front());
        v.pop_front();
        return t;
    }
    const T& peek() {
        return v.front();
    }
    template<typename U>
    void putback(U&& x) {
        v.push_back(std::forward<U>(x));
    }
    template<typename... Args>
    void emplace(Args&&... args) {
        v.emplace_back(std::forward<Args>(args)...);
    }
    bool eof() {
        return v.empty();
    }
    bool fail() {
        return false;
    }
    std::deque<T> v;
};

} // namespace utils (exported)