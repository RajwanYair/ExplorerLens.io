#pragma once

#include <cassert>
#include <cmath>

#ifndef TEST
#define TEST(testSuiteName, testName) static void testSuiteName##_##testName()
#endif

#ifndef EXPECT_TRUE
#define EXPECT_TRUE(condition) assert((condition))
#endif
#ifndef EXPECT_FALSE
#define EXPECT_FALSE(condition) assert(!(condition))
#endif
#ifndef EXPECT_EQ
#define EXPECT_EQ(lhs, rhs) assert(((lhs) == (rhs)))
#endif
#ifndef EXPECT_NE
#define EXPECT_NE(lhs, rhs) assert(((lhs) != (rhs)))
#endif
#ifndef EXPECT_LT
#define EXPECT_LT(lhs, rhs) assert(((lhs) < (rhs)))
#endif
#ifndef EXPECT_LE
#define EXPECT_LE(lhs, rhs) assert(((lhs) <= (rhs)))
#endif
#ifndef EXPECT_GT
#define EXPECT_GT(lhs, rhs) assert(((lhs) > (rhs)))
#endif
#ifndef EXPECT_GE
#define EXPECT_GE(lhs, rhs) assert(((lhs) >= (rhs)))
#endif
#ifndef EXPECT_DOUBLE_EQ
#define EXPECT_DOUBLE_EQ(lhs, rhs) assert((std::fabs((lhs) - (rhs)) <= 1e-12))
#endif
#ifndef EXPECT_FLOAT_EQ
#define EXPECT_FLOAT_EQ(lhs, rhs) assert((std::fabs((lhs) - (rhs)) <= 1e-6f))
#endif
#ifndef EXPECT_NO_THROW
#define EXPECT_NO_THROW(statement) do { try { statement; } catch (...) { assert(!"Expected no exception"); } } while (0)
#endif
#ifndef ASSERT_TRUE
#define ASSERT_TRUE(condition) assert((condition))
#endif
#ifndef ASSERT_FALSE
#define ASSERT_FALSE(condition) assert(!(condition))
#endif
#ifndef ASSERT_EQ
#define ASSERT_EQ(lhs, rhs) assert(((lhs) == (rhs)))
#endif
#ifndef ASSERT_NE
#define ASSERT_NE(lhs, rhs) assert(((lhs) != (rhs)))
#endif
