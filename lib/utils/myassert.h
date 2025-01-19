#pragma once

#ifndef NDEBUG

#ifdef GTEST_ASSERT
#include <gtest/gtest.h>
#define MY_ASSERT EXPECT_TRUE
#else
#include <cassert>
#define MY_ASSERT assert
#endif

#else
#define MY_ASSERT(x)
#endif
