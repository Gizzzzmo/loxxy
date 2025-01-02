#include <cstdint>
#include <gtest/gtest.h>
#include <thread>
#include <chrono>

import utils.tqstream;
import utils.murmurhash;

using utils::tqstream;
using utils::MurmurHash64A;

using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::milliseconds;

static volatile uint64_t out;
#define RUN_TIME 1024*1024*16
static constexpr int n_hashes = 2;
static constexpr size_t buffer_size = 1024;

TEST(TQStream, HashPipeline) { 
    tqstream<uint64_t> stream{1024*32, buffer_size};

    duration<double, std::milli> diff_1;
    duration<double, std::milli> diff_2;

    auto t1 = high_resolution_clock::now();
    std::thread part1([&]() {
        auto t1 = high_resolution_clock::now();
        for (uint64_t i = 0; i < RUN_TIME; i++) {
            volatile uint64_t blub = i;
            for(int i = 0; i < n_hashes; i++) 
                blub = MurmurHash64A(nullptr, 0, blub);
            stream.putback(blub + 0);
        }
        stream.flush();
        auto t2 = high_resolution_clock::now();
        diff_1 = t2 - t1;
    });
    std::thread part2([&]() {

        auto t1 = high_resolution_clock::now();
        for (uint64_t i = 0; i < RUN_TIME; i++) {
            volatile uint64_t blub = stream.get();
            for(int i = 0; i < n_hashes; i++) 
                blub = MurmurHash64A(nullptr, 0, blub);
            out = blub;
        }
        auto t2 = high_resolution_clock::now();
        diff_2 = t2 - t1;
    });
    part1.join();
    part2.join();
    auto t2 = high_resolution_clock::now();

    duration<double, std::milli> ms_double = t2 - t1;
    std::cout << ms_double.count() << "\n  " << diff_1.count() << "\n  " << diff_2.count() << "\n";

}

TEST(TQStream, HashSimple) {

    tqstream<uint64_t> stream{RUN_TIME + buffer_size, buffer_size};
    auto t1 = high_resolution_clock::now();
    
    for (uint64_t i = 0; i < RUN_TIME; i++) {
        volatile uint64_t blub = i;
        for(int i = 0; i < n_hashes; i++) 
            blub = MurmurHash64A(nullptr, 0, blub);
        stream.putback(blub + 0);
    }
    stream.flush();

    for (uint64_t i = 0; i < RUN_TIME; i++) {
        volatile uint64_t blub = stream.get();
        for(int i = 0; i < n_hashes; i++) 
            blub = MurmurHash64A(nullptr, 0, blub);
        out = blub;
    }
    auto t2 = high_resolution_clock::now();

    duration<double, std::milli> ms_double = t2 - t1;
    std::cout << ms_double.count() << std::endl;
}


