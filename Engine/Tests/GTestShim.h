#pragma once

// Compatibility shim providing Google Test-style macros that always evaluate
// their conditions (even in Release builds). Uses throw-based assertions
// matching the main test framework's ASSERT() pattern so MSVC does not
// optimize away variables used only in these macros (avoids C4189).
// TEST() uses [[maybe_unused]] so MSVC does not warn C4505 when a test
// function defined in one TU is not called in that same TU.

#include <cmath>
#include <cstring>

#ifndef TEST
    #define TEST(testSuiteName, testName) [[maybe_unused]] static void testSuiteName##_##testName()
#endif
#ifndef EXPECT_TRUE
    #define EXPECT_TRUE(condition) \
        do { if (!(condition)) throw "EXPECT_TRUE(" #condition ") failed"; } while (0)
#endif
#ifndef EXPECT_FALSE
    #define EXPECT_FALSE(condition) \
        do { if ((condition)) throw "EXPECT_FALSE(" #condition ") failed"; } while (0)
#endif
#ifndef EXPECT_EQ
    #define EXPECT_EQ(lhs, rhs) \
        do { if (!((lhs) == (rhs))) throw "EXPECT_EQ(" #lhs ", " #rhs ") failed"; } while (0)
#endif
#ifndef EXPECT_NE
    #define EXPECT_NE(lhs, rhs) \
        do { if ((lhs) == (rhs)) throw "EXPECT_NE(" #lhs ", " #rhs ") failed"; } while (0)
#endif
#ifndef EXPECT_LT
    #define EXPECT_LT(lhs, rhs) \
        do { if (!((lhs) < (rhs))) throw "EXPECT_LT(" #lhs ", " #rhs ") failed"; } while (0)
#endif
#ifndef EXPECT_LE
    #define EXPECT_LE(lhs, rhs) \
        do { if (!((lhs) <= (rhs))) throw "EXPECT_LE(" #lhs ", " #rhs ") failed"; } while (0)
#endif
#ifndef EXPECT_GT
    #define EXPECT_GT(lhs, rhs) \
        do { if (!((lhs) > (rhs))) throw "EXPECT_GT(" #lhs ", " #rhs ") failed"; } while (0)
#endif
#ifndef EXPECT_GE
    #define EXPECT_GE(lhs, rhs) \
        do { if (!((lhs) >= (rhs))) throw "EXPECT_GE(" #lhs ", " #rhs ") failed"; } while (0)
#endif
#ifndef EXPECT_DOUBLE_EQ
    #define EXPECT_DOUBLE_EQ(lhs, rhs) \
        do { if (std::fabs((lhs) - (rhs)) > 1e-12) throw "EXPECT_DOUBLE_EQ(" #lhs ", " #rhs ") failed"; } while (0)
#endif
#ifndef EXPECT_FLOAT_EQ
    #define EXPECT_FLOAT_EQ(lhs, rhs) \
        do { if (std::fabs((lhs) - (rhs)) > 1e-6f) throw "EXPECT_FLOAT_EQ(" #lhs ", " #rhs ") failed"; } while (0)
#endif
#ifndef EXPECT_NO_THROW
    #define EXPECT_NO_THROW(statement)                    \
        do {                                              \
            try {                                         \
                statement;                                \
            } catch (...) {                               \
                throw "EXPECT_NO_THROW: unexpected exception thrown"; \
            }                                             \
        } while (0)
#endif
#ifndef ASSERT_TRUE
    #define ASSERT_TRUE(condition) \
        do { if (!(condition)) throw "ASSERT_TRUE(" #condition ") failed"; } while (0)
#endif
#ifndef ASSERT_FALSE
    #define ASSERT_FALSE(condition) \
        do { if ((condition)) throw "ASSERT_FALSE(" #condition ") failed"; } while (0)
#endif
#ifndef ASSERT_EQ
    #define ASSERT_EQ(lhs, rhs) \
        do { if (!((lhs) == (rhs))) throw "ASSERT_EQ(" #lhs ", " #rhs ") failed"; } while (0)
#endif
#ifndef ASSERT_NE
    #define ASSERT_NE(lhs, rhs) \
        do { if ((lhs) == (rhs)) throw "ASSERT_NE(" #lhs ", " #rhs ") failed"; } while (0)
#endif
#ifndef ASSERT
    #define ASSERT(cond) \
        do { if (!(cond)) throw "ASSERT(" #cond ") failed"; } while (0)
#endif
#ifndef FAIL
    #define FAIL() throw "Test failed"
#endif
#ifndef EXPECT_STREQ
    #define EXPECT_STREQ(s1, s2) \
        do { if (std::strcmp((s1), (s2)) != 0) throw "EXPECT_STREQ(" #s1 ", " #s2 ") failed"; } while (0)
#endif
#ifndef EXPECT_STRNE
    #define EXPECT_STRNE(s1, s2) \
        do { if (std::strcmp((s1), (s2)) == 0) throw "EXPECT_STRNE(" #s1 ", " #s2 ") failed"; } while (0)
#endif
#ifndef SUCCEED
    #define SUCCEED() ((void)0)
#endif
