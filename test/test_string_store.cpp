#include <gtest/gtest.h>

import utils.string_store;

using namespace utils;

TEST(StringStore, EmptyStore) {
    persistent_string_store store;
    EXPECT_EQ(store.finish_recording(), nullptr);
    EXPECT_EQ(store.begin(), store.end());
    EXPECT_EQ(store.cbegin(), store.cend());
}

TEST(StringStore, RecordChar) {
    persistent_string_store store;
    EXPECT_TRUE(store.start_recording());
    EXPECT_TRUE(store.recordChar('a'));
    const persistent_string<char>* string = store.finish_recording();

    EXPECT_EQ(string->len, 1);
    EXPECT_EQ(string->chars[0], 'a');
}

TEST(StringStore, RecordString) {
    persistent_string_store store;
    EXPECT_TRUE(store.start_recording());
    EXPECT_TRUE(store.recordString("Hello World"));
    const persistent_string<char>* string = store.finish_recording();

    EXPECT_EQ(string->len, 11);
    EXPECT_EQ(std::string_view(*string), "Hello World");
}

TEST(StringStore, Exhaustive) {
    persistent_string_store store;

    for (int i = 0; i < 32; i++) {
        store.start_recording();
        for (int j = 0; j < i; j++)
            store.recordString("blib");
        store.finish_recording();
    }
    int i = 0;
    for (const auto& str : store)  {
        EXPECT_EQ(str.len, i * 4);
        for (int j = 0; j < str.len; j+=4) {
            EXPECT_EQ(str.chars[j], 'b');
            EXPECT_EQ(str.chars[j + 1], 'l');
            EXPECT_EQ(str.chars[j + 2], 'i');
            EXPECT_EQ(str.chars[j + 3], 'b');
        }
        i++;
    }
}