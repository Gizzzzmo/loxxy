module;

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <new>
#include <ostream>
#include <string_view>
#include <type_traits>
#include <vector>
#include "myassert.h"

export module utils.string_store;

export namespace utils {

using size_tt = uint32_t;

template<typename char_t = char>
class persistent_string {
    private:
        persistent_string(size_tt len) : len(len) {};
        persistent_string(const persistent_string&) = delete;
        persistent_string(persistent_string&&) = delete;
    public:
        static auto allocate_new(size_tt capacity) {
            std::byte* memory = 
                new(std::align_val_t{alignof(persistent_string<char_t>)})
                std::byte[sizeof(persistent_string<char_t>) + capacity * sizeof(char_t)];

            return std::unique_ptr<persistent_string<char_t>>(construct_at(memory));
        }
        static persistent_string<char_t>* construct_at(void* ptr) {
            static_assert(std::is_trivially_destructible_v<persistent_string<char_t>> ==  std::is_trivially_destructible_v<char_t>);
            return new (ptr) persistent_string<char_t>{0};
        }
        persistent_string<char_t>* copy_to(void* ptr) requires (std::is_copy_constructible_v<char_t>) {
            persistent_string<char_t>* str = construct_at(ptr);
            str->len = len;
            if constexpr (std::is_trivially_copyable_v<char_t>)
                std::memcpy(str->chars, chars, sizeof(char_t) * len);
            else {
                for (size_tt i; i < len; i++) {
                    new (str->chars[i]) char_t(chars[i]);
                }
            }
            return str;
        }
        persistent_string<char_t>* move_to(void *ptr) requires (std::is_move_constructible_v<char_t>) {
            persistent_string<char_t>* str = construct_at(ptr);
            str->len = len;
            if constexpr (std::is_trivially_move_constructible_v<char_t>)
                std::memcpy(str->chars, chars, sizeof(char_t) * len);
            else {
                for (size_tt i; i < len; i++) {
                    new (str->chars[i]) char_t(std::move(chars[i]));
                }
            }
            return str;
        }

        void destruct_chars() {
            if constexpr (!std::is_trivially_destructible_v<char_t>) {
                for (size_tt i; i < len; i++) {
                    chars[i].~char_t();
                }
            }
            len = 0;
        }

        ~persistent_string() requires (!std::is_trivially_destructible_v<char_t>){
            for (size_tt i; i < len; i++) {
                chars[i].~char_t();
            }
        }
        ~persistent_string() requires (std::is_trivially_destructible_v<char_t>) = default;

        operator std::basic_string_view<char_t>() const {
            return std::basic_string_view<char_t>(chars, len);
        }
        size_t byte_size() const {
            return sizeof(persistent_string<char_t>) + len * sizeof(char_t);
        }

        friend std::ostream& operator<<(std::ostream& ostream, const persistent_string<char_t>& string) {
            ostream << std::basic_string_view<char_t>(string);
            return ostream;
        }
        size_tt len;
        char_t chars[];
};

template<typename char_tt = char, size_tt chunk_size = 1<<10>
class persistent_string_store {
    using char_t = char;
    using param_char_t = 
        std::conditional_t<
            std::is_trivially_copyable_v<char_t> && sizeof(char_t) <= sizeof(size_tt),
            char_t, const char_t&
        >;

    static constexpr size_tt chunk_size_in_persistent_strings =
        (chunk_size * sizeof(char_t) + sizeof(persistent_string<char_t>) - 1) / sizeof(persistent_string<char_t>);

    static constexpr size_tt chunk_size_in_bytes = chunk_size_in_persistent_strings * sizeof(persistent_string<char_t>);

    public:

        class string_store_iterator {
            friend class persistent_string_store<char_t, chunk_size>;

            string_store_iterator(
                const std::vector<std::byte*>& container,
                const size_tt& byte_size
            ) : container(container),
                string(reinterpret_cast<persistent_string<char_t>*>(container.front())),
                byte_size(byte_size),
                chunk_index(0) { }

            string_store_iterator(const std::vector<std::byte*>& container)
            : container(container), byte_size(chunk_index), chunk_index(std::numeric_limits<size_tt>::max()) { }

            public:
                string_store_iterator& operator++() {
                    if (is_end())
                        return *this;

                    const char_t* address = string->chars;
                    address += string->len;
                    size_t pos_in_chunk = reinterpret_cast<size_t>(address) - reinterpret_cast<size_t>(container[chunk_index]);
                    
                    if (chunk_index == container.size() - 1 && pos_in_chunk >= byte_size) {
                        MY_ASSERT(pos_in_chunk = byte_size);
                        chunk_index = std::numeric_limits<size_tt>::max();
                        return *this;
                    }

                    size_t space = alignof(persistent_string<char_t>);
                    void* ptr = const_cast<char*>(address);

                    const void* next_address = std::align(
                        alignof(persistent_string<char_t>),
                        0,
                        ptr, space
                    );
                    MY_ASSERT(next_address != nullptr);
                    
                    string = reinterpret_cast<const persistent_string<char_t>*>(next_address);

                    if (string->len == std::numeric_limits<size_tt>::max()) {
                        chunk_index++;
                        if (chunk_index >= container.size())
                            chunk_index = std::numeric_limits<size_tt>::max();
                        else
                            string = reinterpret_cast<persistent_string<char_t>*>(container[chunk_index]);
                    }

                    return *this;
                }

                const persistent_string<char_t>& operator*() const {
                    return *string;
                }

                bool operator==(const string_store_iterator& other) const {
                    if (&container != &other.container)
                        return false;

                    if (is_end() && other.is_end())
                        return true;

                    return string == other.string && chunk_index == other.chunk_index;
                }

                bool operator!=(const string_store_iterator& other) const {
                    return !(*this == other);
                }

            private:
                bool is_end() const {
                    return chunk_index == std::numeric_limits<size_tt>::max();
                }
                const std::vector<std::byte*>& container;
                const persistent_string<char_t>* string;
                const size_tt& byte_size;
                size_tt chunk_index;
        };

        persistent_string_store() {}
        ~persistent_string_store() {
            if constexpr (!std::is_trivially_destructible_v<persistent_string<char_t>>) {
                for (const persistent_string<char_t>& string : *this) {
                    string.~persistent_string<char_t>();
                }
            }
            
            for (const std::byte* chunk : memory) {
                delete[] chunk;
            }
        }

        bool start_recording(size_tt min_num_chars = 0) {
            if (recording_string)
                return false;
            recording_string = true;

            if (current_recording == nullptr) {
                MY_ASSERT(size_bytes == 0);
                MY_ASSERT(capacity_bytes == 0);
                allocate_new_chunks(min_num_chars);
                current_recording = persistent_string<char_t>::construct_at(memory.back());
                size_bytes += sizeof(persistent_string<char_t>);
                return true;
            }

            void* free_space = &current_recording->chars[current_recording->len];
            size_t space = alignof(persistent_string<char_t>);
            void *aligned = std::align(alignof(persistent_string<char_t>), 0, free_space, space);

            size_tt alignment_space = alignof(persistent_string<char_t>) - space;

            size_tt min_required_space = alignment_space
                + sizeof(persistent_string<char_t>)
                + min_num_chars * sizeof(char_t);
            
            if (min_required_space > capacity_bytes - size_bytes) {
                allocate_new_chunks(min_num_chars);
                if (aligned != nullptr)
                    persistent_string<char_t>::construct_at(aligned)->len = std::numeric_limits<size_tt>::max();
                aligned = memory.back();
            }
            else
                size_bytes += alignment_space;

            size_bytes += sizeof(persistent_string<char_t>);

            current_recording = persistent_string<char_t>::construct_at(aligned);

            return true;
        }

        const persistent_string<char_t>* finish_recording() {
            recording_string = false;
            return current_recording;
        }

        const persistent_string<char_t>* peek_recording() const {
            return current_recording;
        }

        void reset_recording() {
            if(!recording_string)
                return;
            
            size_bytes -= current_recording->len * sizeof(char_t);
            current_recording->destruct_chars();

            return;
        }

        bool recordChar(param_char_t c) {
            if (!recording_string)
                return false;

            while (sizeof(char_t) > capacity_bytes - size_bytes)
                allocate_and_move_current(1);

            new (&current_recording->chars[current_recording->len]) char_t(c);

            current_recording->len++;
            size_bytes += sizeof(char_t);

            MY_ASSERT(size_bytes == reinterpret_cast<std::byte*>(&current_recording->chars[current_recording->len]) - memory.back());
            return true;
        }

        bool recordChar(char_t&& c) requires (!std::same_as<char_t, param_char_t>){
            if (!recording_string)
                return false;

            if (sizeof(char_t) > capacity_bytes - size_bytes)
                allocate_and_move_current(1);

            new (&current_recording->chars[current_recording->len]) char_t(std::move(c));
            current_recording->len++;
            size_bytes += sizeof(char_t);

            MY_ASSERT(size_bytes == reinterpret_cast<std::byte*>(&current_recording->chars[current_recording->len]) - memory.back());
            return true;
        }

        bool recordString(const std::basic_string_view<char_t>& string) {
            if (!recording_string)
                return false;

            if (sizeof(char_t) * string.size() > capacity_bytes - size_bytes)
                allocate_and_move_current(string.size());

            if constexpr (std::is_trivially_copyable_v<char_t>) {
                std::memcpy(&current_recording->chars[current_recording->len], string.data(), sizeof(char_t) * string.size());
                current_recording->len += string.size();
            } else {
                for (const char_t c : string) {
                    new (&current_recording->chars[current_recording->len]) char_t(c);
                    current_recording->len++;
                }
            }

            size_bytes += sizeof(char_t) * string.size();

            MY_ASSERT(size_bytes == reinterpret_cast<std::byte*>(&current_recording->chars[current_recording->len]) - memory.back());
            return true;
        }

        template<typename... Args>
        bool emplaceChar(Args&&... args) {
            if (!recording_string)
                return false;

            if (sizeof(char_t) > capacity_bytes - size_bytes)
                allocate_and_move_current();

            new (&current_recording->chars[current_recording->len]) char_t(std::forward<Args...>(args...));
            current_recording->len++;
            size_bytes += sizeof(char_t);

            MY_ASSERT(size_bytes == reinterpret_cast<std::byte*>(&current_recording->chars[current_recording->len]) - memory.back());
            return true;
        }

        string_store_iterator cbegin() const {
            if (size_bytes == 0)
                return string_store_iterator(memory);

            return string_store_iterator(memory, size_bytes);
        }

        string_store_iterator cend() const {
            return string_store_iterator(memory);
        }

        auto begin() const {
            return cbegin();
        }

        auto end() const {
            return cend();
        }


    private:
        void allocate_new_chunks(size_t min_num_chars) {
            size_bytes = 0;
            if (capacity_bytes == 0)
                capacity_bytes = chunk_size_in_bytes;
            else
                capacity_bytes *= 2;

            while(capacity_bytes < sizeof(persistent_string<char_t>) + min_num_chars * sizeof(char_t))
                capacity_bytes *= 2;

            memory.push_back(
                new(std::align_val_t{alignof(persistent_string<char_t>)})
                std::byte[capacity_bytes + sizeof(persistent_string<char_t>) + alignof(persistent_string<char_t>)]
            );
        }
        void allocate_and_move_current(size_t min_num_chars) {
            min_num_chars += current_recording->len;
            std::byte* chunk_to_remove = nullptr;

            if (memory.size() >= 1 && reinterpret_cast<std::byte*>(current_recording) == memory.back()) {
                chunk_to_remove = memory.back();
                memory.pop_back();
            }
            allocate_new_chunks(min_num_chars);

            persistent_string<char_t>* moved = current_recording->move_to(memory.back());
            size_bytes += moved->byte_size();
            current_recording->destruct_chars();
            
            if (chunk_to_remove == nullptr)
                current_recording->len = std::numeric_limits<size_tt>::max();
            else
                delete[] chunk_to_remove;

            current_recording = moved;
        }
        size_tt capacity_bytes = 0;
        size_tt size_bytes = 0;

        bool recording_string = false;
        persistent_string<char_t>* current_recording = nullptr;
        std::vector<std::byte*> memory;
};

} // namespace utils