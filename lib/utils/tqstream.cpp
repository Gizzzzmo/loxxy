module;

#include "rigtorp/SPSCQueue.h"
#include <cassert>
#include <chrono>
#include <thread>

export module utils.tqstream;

export namespace utils {

template <typename T, typename FlushPred = bool (*)(const T&)>
class tqstream {
public:
    tqstream(
        size_t n, size_t buffer_size_in = 1, FlushPred flush_pred = [](const T&) { return false; }
    )
        : buffer_size_in(buffer_size_in), queue(n), flush_pred(flush_pred) {}

    auto get() -> T {
        T* ptr;
        while ((ptr = queue.front()) == nullptr)
            std::this_thread::sleep_for(std::chrono::microseconds(250));

        T el = *ptr;
        queue.pop();

        return el;
    }

    auto peek() -> const T& {
        T* ptr;
        while ((ptr = queue.front()) == nullptr)
            std::this_thread::sleep_for(std::chrono::microseconds(250));

        return *ptr;
    }

    void putback(T&& x) {
        T* built = queue.build(std::move(x));
        if (built == nullptr) {

            size_t n_finished = queue.finish_build(buffer_size_in);
            assert(n_finished == buffer_size_in || flushed);
            bool success = queue.wait_reserve_build(buffer_size_in);
#ifndef NDEBUG
            flushed = false;
#endif
            assert(success);
            built = queue.build(std::move(x));
            assert(built != nullptr);
        }
        if (flush_pred(*built))
            flush();
    }

    template <typename... Args>
    void emplace(Args&&... args) {
        T* built = queue.build(std::forward<Args>(args)...);
        if (built == nullptr) {
            size_t n_finished = queue.finish_build(buffer_size_in);
            assert(n_finished == buffer_size_in || flushed);
            bool success = queue.wait_reserve_build(buffer_size_in);
#ifndef NDEBUG
            flushed = false;
#endif
            assert(success);
            built = queue.build(std::forward<Args>(args)...);
            assert(built != nullptr);
        }
        if (flush_pred(*built))
            flush();
    }

    void flush() {
#ifndef NDEBUG
        flushed = true;
#endif
        queue.finish_build(buffer_size_in);
    }

private:
    size_t last_size;
    size_t buffer_size_in;
    rigtorp::SPSCQueue<T> queue;
    FlushPred flush_pred;
#ifndef NDEBUG
    bool flushed = true;
#endif
};

} // namespace utils
