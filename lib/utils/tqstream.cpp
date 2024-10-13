module;

#include "rigtorp/SPSCQueue.h"
#include <thread>

export module utils.tqstream;

export namespace utils {

template<typename T>
class tqstream {
    public:
        tqstream(size_t n) : queue(n), need_to_pop(false) {}
        T&& get() {
            if (need_to_pop)
                queue.pop();

            T* ptr;
            while((ptr = queue.front()) == nullptr)
                std::this_thread::yield();
            
            need_to_pop = true;

            return std::move(*ptr);
        }
        const T& peek() {
            if (need_to_pop)
                queue.pop();

            T* ptr;
            while((ptr = queue.front()) == nullptr)
                std::this_thread::yield();

            need_to_pop = false;

            return *ptr;
        }
        void putback(T&& x) {
            queue.push(std::move(x));
        }

        template<typename... Args>
        void emplace(Args&&... args) {
            queue.emplace(std::forward<Args>(args)...);
        }

    private:
        rigtorp::SPSCQueue<T> queue;
        bool need_to_pop;
};

} // namespace utils