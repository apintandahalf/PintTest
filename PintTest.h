#ifndef PINT_TEST_H
#define PINT_TEST_H

/*
A very basic C++20 header only unit test library.
MIT Licence.
apintandahalf would appreciate attribution.

The following expects and asserts are supported (assert is the same as expect but will return immediately from the test case):

    {EXPECT|ASSERT}_TRUE(a)
    {EXPECT|ASSERT}_FALSE(a)
    {EXPECT|ASSERT}_EQ(a,b)
    {EXPECT|ASSERT}_NE(a,b)
    {EXPECT|ASSERT}_GT(a,b)
    {EXPECT|ASSERT}_LT(a,b)
    {EXPECT|ASSERT}_GE(a,b)
    {EXPECT|ASSERT}_LE(a,b)
    {EXPECT|ASSERT}_NEAR(a, b, tol)

There is nothing to build  - just include the header sraight into your test code.
Test cases are auto-registered across all cpp files in the test program.
PintTest::runAllTests will run them all and report the detail and a summary.

Run time parameters:

--filter=-<some_string>
    If the test name matches some_string (simply, no regex supported) then don't run the test.If not, do.  If not specified, run all tests.
    Note that filtering for only one string is supported

--filter=<some_string>
    If the test name matches some_string *(simply, no regex supported) then run the test.If not, not.  If not specified, run all tests.
    Note that filtering for only one string is supported

You can't specify both the filter and the not filter.

Example use:

#include <PintTest.h>

int times2(int x) // function under test
{
    return x * 2;
}
TEST(testtimes2)
{
    EXPECT_EQ(4, times2(2));
    ASSERT_EQ(6, times2(3));
    for (int i=0; i<3; ++i)
        ASSERT_EQ(i*2, times2(i)) << i;
}
TEST(testtimes2Wrong)
{
    ASSERT_NE(7, times2(3));
    EXPECT_NE(5, times2(2));
    for (int i=1; i<3; ++i)
        ASSERT_NE(0, times2(i)) << i;
}
int main()
{
    return PintTest::runAllTests();
}

*/

#include <iostream>
#include <source_location>
#include <functional>
#include <vector>
#include <sstream>
#include <chrono>

#ifdef _WIN32
#include "Windows.h"
#endif

#include <unordered_set>
#include <vector>
#include <string>
#include <string_view>
#include <functional>

class PintTest
{
    struct Test
    {
        std::string m_name;
        std::function<void()> m_fn;
        void test(const std::string& filter, const std::string& notFilter)
        {
            if (!notFilter.empty() && m_name.find(notFilter) != std::string::npos)
                return;

            if (filter.empty() || m_name.find(filter) != std::string::npos)
                PintTest::runTest(m_fn, m_name);
        }
    };
    static inline std::vector<Test> tests;
    static inline std::unordered_set<std::string> test_names;

public:

    static inline int s_fails = 0;
    static inline int s_tests_failed = 0;
    static inline int s_tests_ran = 0;

    static constexpr auto GREEN_TEXT_START = "\x1B[32m";
    static constexpr auto RED_TEXT_START = "\x1B[31m";
    static constexpr auto COLOUR_TEXT_END = "\033[0m\t\t";

    static bool registerTestFn(const char* name, std::function<void()> test)
    {
        if (test_names.contains(name))
        {
            std::cout << RED_TEXT_START << name << name << " has already been registered" << COLOUR_TEXT_END << "\n";
            exit(1);
        }
        test_names.insert(name);
        tests.push_back({ name, test });
        return true;
    }

    static void selfTest();

    // Create from argc/argv as follows: std::vector<std::string_view> args(argv, argv + argc);
    // returns a pair of ints - the number of tests ran, and the number of tests failed.  If first is negative, then the arguments are invalid
    static std::pair<int,int> runAllTests2(const std::vector<std::string_view>& args)
    {
        s_fails = 0;
        s_tests_failed = 0;
        s_tests_ran = 0;

        PintTest::runTest(PintTest::selfTest, "PintTest::selfTest");
        if (s_fails > 0)
            return { s_tests_ran, s_fails };

        std::string filter;
        std::string notFilter;
        if (!args.empty())
        {
            std::string filterArg = "--filter=";
            std::string notFilterArg = "--filter=-";
            for (auto& arg : args)
            {
                if (arg.starts_with(filterArg))
                {
                    if (!filter.empty() || !notFilter.empty())
                    {
                        std::cerr << "More than one instance of \"--filter=\", terminating\n";
                        return { -1, 1 };
                    }
                    if (arg.size() == filterArg.size())
                    {
                        std::cerr << "\"--filter=\" specified without a filter, nothing to do\n";
                        return { s_tests_ran, s_fails };
                    }
                    if (arg.size() == notFilterArg.size() && arg[arg.size()-1] == '-')
                    {
                        std::cerr << "\"--filter=-\" specified without a filter, terminating\n";
                        return { -1, 2 };
                    }
                    if (arg.starts_with(notFilterArg))
                        notFilter = arg.substr(notFilterArg.length(), arg.size() - notFilterArg.size());
                    else
                        filter = arg.substr(filterArg.length(), arg.size() - filterArg.size());
                }
            }
        }
        const auto start = std::chrono::system_clock::now();

        for (auto& test : tests)
            test.test(filter, notFilter);

        const auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();

        if (s_fails)
            std::cout << RED_TEXT_START << "Ran " << s_tests_ran << " tests and " << s_tests_failed << " failed (" << durationMs << "ms)" << COLOUR_TEXT_END << "\n";
        else
            std::cout << GREEN_TEXT_START << "Ran " << s_tests_ran << " tests and none failed" << " failed (" << durationMs << "ms)" << COLOUR_TEXT_END << "\n";

        return { s_tests_ran, s_fails };
    }
    // Create from argc/argv as follows: std::vector<std::string_view> args(argv, argv + argc);
    static int runAllTests(const std::vector<std::string_view>& args = std::vector<std::string_view>{})
    {
        auto[_, failed] = runAllTests2(args);
        return failed;
    }

    static void runTest(std::function<void()> testFn, const std::string& testCase)
    {
        ++PintTest::s_tests_ran;
        const auto currentFails = PintTest::s_fails;
        std::cout << PintTest::GREEN_TEXT_START << "Testing " << testCase << PintTest::COLOUR_TEXT_END << "\n";
        const auto start = std::chrono::system_clock::now();
        testFn();
        const auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start).count();
        if (currentFails < PintTest::s_fails)
        {
            ++PintTest::s_tests_failed;
            std::cout << PintTest::RED_TEXT_START << "FAILED  " << testCase << " (" << durationMs << "ms)" << PintTest::COLOUR_TEXT_END << "\n";
        }
        else
        {
            std::cout << PintTest::GREEN_TEXT_START << "PASSED  " << testCase << " (" << durationMs << "ms)" << PintTest::COLOUR_TEXT_END << "\n";
        }
    }
};

namespace PintTestNS
{
    // GetString returns the stream version of the object if streaming is supported, otherwise just returns the address
    //@todo - maybe do something about const char*
    template <typename T> concept Streamable = requires(std::ostringstream os, T value) { { os << value }; };
    template <typename T> requires (Streamable<T>)
        inline auto constexpr GetString(const T& value)
    {
        std::ostringstream ss;
        ss << value;
        return ss.str();
    }
    template <typename T> requires (!Streamable<T>)
        inline auto constexpr GetString(const T& value)
    {
        return to_string(&value);
    }

    // Captures the error msessage, for when the test fails
    class Msg
    {
    public:
        Msg() = default;
        Msg(const std::string& value)
            : m_Empty(value.empty())
        {
            m_SS << value;
        }
        template <class T>
        Msg& operator<<(const T& value)
        {
            m_SS << GetString(value);
            return *this;
        }
        friend std::ostream& operator<<(std::ostream& ss, const Msg& value)
        {
            ss << value.getString();
            return ss;
        }
        void clear()
        {
            m_SS.clear();
        }
        [[nodiscard]] std::string getString() const { return m_SS.str(); }
        [[nodiscard]] bool empty() const { return m_Empty; }
    private:
        bool m_Empty = true;
        std::ostringstream m_SS;

    };

    // function to create the error string (not including any extra values passed in via <<)
    template<class L, class R>
    Msg createCompareString(bool pass, const char* prefix, const L& l, const R& r, const char* pL, const char* pR)
    {
        if (pass)
            return {};
        PintTest::s_fails += 1;
        return std::string("\n") + pL + " " + prefix + " " + pR + "\nExpected: " + GetString(l) + "\n" + "Actual: " + GetString(r) + "\n";
    }

    inline Msg createCompareStringNear(bool pass, double l, double r, const char* pL, const char* pR, const char* pTolerance)
    {
        if (pass)
            return {};
        PintTest::s_fails += 1;
        return std::string("\n") + pL + " == " + pR + " (+/-" + pTolerance + ")\nExpected: " + GetString(l) + "\n" + "Actual: " + GetString(r) + "\n";
    }

    // Test true - return an empty Msg if true, or a Msg with the error message in it if false
    template<class T>
    Msg compTrue(const T& t, const char* pT)
    {
        return createCompareString((t), "is true", t, true, pT, "true");
    }
    // Test false - return an empty Msg if false, or a Msg with the error message in it if true
    template<class T>
    Msg compFalse(const T& t, const char* pT)
    {
        return createCompareString(!(t), "is false", t, false, pT, "false");
    }

    // Test equals - return an empty Msg if pass, or a Msg with the error message in it if failed
    template<class L, class R>
    Msg compEq(const L& l, const R& r, const char* pL, const char* pR)
    {
        return createCompareString((l == r), "==", l, r, pL, pR);
    }
    // Test not equals - return an empty Msg if pass, or a Msg with the error message in it if failed
    template<class L, class R>
    Msg compNe(const L& l, const R& r, const char* pL, const char* pR)
    {
        return createCompareString(!(l == r), "!=", l, r, pL, pR);
    }

    // Test geater than - return an empty Msg if pass, or a Msg with the error message in it if failed
    template<class L, class R>
    Msg compGt(const L& l, const R& r, const char* pL, const char* pR)
    {
        return createCompareString((l > r), ">", l, r, pL, pR);
    }
    // Test less than - return an empty Msg if pass, or a Msg with the error message in it if failed
    template<class L, class R>
    Msg compLt(const L& l, const R& r, const char* pL, const char* pR)
    {
        return createCompareString((l < r), "<", l, r, pL, pR);
    }
    // Test geater than or equal to - return an empty Msg if pass, or a Msg with the error message in it if failed
    template<class L, class R>
    Msg compGe(const L& l, const R& r, const char* pL, const char* pR)
    {
        return createCompareString((l >= r), ">=", l, r, pL, pR);
    }
    // Test less than or equal to - return an empty Msg if pass, or a Msg with the error message in it if failed
    template<class L, class R>
    Msg compLe(const L& l, const R& r, const char* pL, const char* pR)
    {
        return createCompareString((l <= r), "<=", l, r, pL, pR);
    }

    // Test for nearness with doubles/floats - return an empty Msg if pass, or a Msg with the error message in it if failed
    inline Msg compNear(double l, double r, double tolerance, const char* pL, const char* pR, const char* pTolerance)
    {
        const double diff = l > r ? l - r : r - l;
        return createCompareStringNear(diff < tolerance, l, r, pL, pR, pTolerance);
    }

    // a class to actually output the error message, and add a new line of required
    class MsgWriter
    {
    public:

        MsgWriter(const char* fn, const char* func, uint_least32_t line)
            : m_fn(fn), m_func(func), m_line(line)
        {
        }
        void operator=(Msg& str)
        {
            m_Str = str.getString();
            str.clear();
        }
        ~MsgWriter()
        {
            {
                if (!m_Str.empty() && m_Str[m_Str.length() - 1] == '\n')
                    m_Str = m_Str.substr(0, m_Str.length() - 1);
                std::ostringstream os_;
                os_ << m_fn << "(" << m_line << "): " << m_Str;
                std::cout
                    << PintTest::RED_TEXT_START
                    << "Test failed: "
                    << os_.str()
                    << PintTest::COLOUR_TEXT_END
                    << "\n";
#ifdef _WIN32
                if (IsDebuggerPresent())
                {
                    auto str = os_.str();
                    str += "\n";
                    OutputDebugString(str.data());
                }
#endif
            }
        }
    private:
        const char* m_fn;
        const char* m_func;
        uint_least32_t m_line;
        std::string m_Str;
    };
}

// Macro to generate a test case
#define TEST(TestCase) \
    namespace \
    { \
        struct TestStruct##TestCase \
        { \
            static void TestBody(); \
        }; \
        static const auto sTestStruct##TestCase = PintTest::registerTestFn(#TestCase, TestStruct##TestCase::TestBody); \
    } \
    void ::TestStruct##TestCase::TestBody()

// Supporting macros for the expects and asserts to extract the commonality
#define CONSTRUCT_WRITER \
    PintTestNS::MsgWriter(std::source_location::current().file_name(), std::source_location::current().function_name(), std::source_location::current().line())

#define PREAMBLE \
    if (auto utestMsg = 

#define POSTAMBLE_EXPECT \
     ; utestMsg.empty()) ; /*no op*/ else CONSTRUCT_WRITER = utestMsg

#define POSTAMBLE_ASSERT \
     ; utestMsg.empty()) ; /*no op*/ else return CONSTRUCT_WRITER = utestMsg

//
//
// These are the expects and asserts that the test code actuals calls
//
//

#define EXPECT_TRUE(a) \
    PREAMBLE PintTestNS::compTrue((a), #a) POSTAMBLE_EXPECT

#define EXPECT_FALSE(a) \
    PREAMBLE PintTestNS::compFalse((a), #a) POSTAMBLE_EXPECT

#define EXPECT_EQ(a,b) \
    PREAMBLE PintTestNS::compEq((a), (b), #a, #b) POSTAMBLE_EXPECT

#define EXPECT_NE(a,b) \
    PREAMBLE PintTestNS::compNe((a), (b), #a, #b) POSTAMBLE_EXPECT

#define EXPECT_GT(a,b) \
    PREAMBLE PintTestNS::compGt((a), (b), #a, #b) POSTAMBLE_EXPECT

#define EXPECT_LT(a,b) \
    PREAMBLE PintTestNS::compLt((a), (b), #a, #b) POSTAMBLE_EXPECT

#define EXPECT_GE(a,b) \
    PREAMBLE PintTestNS::compGe((a), (b), #a, #b) POSTAMBLE_EXPECT

#define EXPECT_LE(a,b) \
    PREAMBLE PintTestNS::compLe((a), (b), #a, #b) POSTAMBLE_EXPECT

#define EXPECT_NEAR(a, b, tol) \
    PREAMBLE PintTestNS::compNear((a), (b), (tol), #a, #b, #tol) POSTAMBLE_EXPECT


#define ASSERT_TRUE(a) \
    PREAMBLE PintTestNS::compTrue((a), #a) POSTAMBLE_ASSERT

#define ASSERT_FALSE(a) \
    PREAMBLE PintTestNS::compFalse((a), #a) POSTAMBLE_ASSERT

#define ASSERT_EQ(a,b) \
    PREAMBLE PintTestNS::compEq((a), (b), #a, #b) POSTAMBLE_ASSERT

#define ASSERT_NE(a,b) \
    PREAMBLE PintTestNS::compNe((a), (b), #a, #b) POSTAMBLE_ASSERT

#define ASSERT_GT(a,b) \
    PREAMBLE PintTestNS::compGt((a), (b), #a, #b) POSTAMBLE_ASSERT

#define ASSERT_LT(a,b) \
    PREAMBLE PintTestNS::compLt((a), (b), #a, #b) POSTAMBLE_ASSERT

#define ASSERT_GE(a,b) \
    PREAMBLE PintTestNS::compGe((a), (b), #a, #b) POSTAMBLE_ASSERT

#define ASSERT_LE(a,b) \
    PREAMBLE PintTestNS::compLe((a), (b), #a, #b) POSTAMBLE_ASSERT

#define ASSERT_NEAR(a, b, tol) \
    PREAMBLE PintTestNS::compNear((a), (b), (tol), #a, #b, #tol) POSTAMBLE_ASSERT

//
//
// End of expects and asserts
//
//

// A function to sanity check the various test macros. These tests should all pass
inline void PintTest::selfTest()
{
    EXPECT_EQ(1, 1);
    EXPECT_NE(1, 2);
    EXPECT_TRUE(1 == 1);
    EXPECT_FALSE(1 == 2);

    ASSERT_EQ(1, 1);
    ASSERT_NE(1, 2);
    ASSERT_TRUE(1 == 1);
    ASSERT_FALSE(1 == 2);

    EXPECT_NEAR(1, 1, 0.01);
    EXPECT_NEAR(1, 1.0001, 0.01);
    EXPECT_NEAR(1.01, 1, 0.1);
    EXPECT_NEAR(100, 101, 10);

    ASSERT_NEAR(1, 1, 0.01);
    ASSERT_NEAR(1, 1.0001, 0.01);
    ASSERT_NEAR(1.01, 1, 0.1);
    ASSERT_NEAR(100, 101, 10);

    EXPECT_GT(2, 1);
    EXPECT_GE(2, 1);
    EXPECT_GE(2, 2);
    EXPECT_LT(1, 2);
    EXPECT_LE(1, 2);
    EXPECT_LE(2, 2);

    ASSERT_GT(2, 1);
    ASSERT_GE(2, 1);
    ASSERT_GE(2, 2);
    ASSERT_LT(1, 2);
    ASSERT_LE(1, 2);
    ASSERT_LE(2, 2);
}


#endif
