#pragma once

#include <functional>
#include <iostream>
#include <string>
#include <vector>

namespace test {

struct TestCase
{
    std::string name;
    std::function<void()> func;
};

inline std::vector<TestCase> &getRegistry()
{
    static std::vector<TestCase> registry;
    return registry;
}

inline bool registerTest(const std::string &name, std::function<void()> func)
{
    getRegistry().push_back({name, std::move(func)});
    return true;
}

struct TestFailure
{
    std::string expression;
    std::string file;
    int line;
    std::string message;
};

inline std::vector<TestFailure> &getCurrentFailures()
{
    static std::vector<TestFailure> failures;
    return failures;
}

inline void recordFailure(const std::string &expr, const std::string &file, int line, const std::string &msg = "")
{
    getCurrentFailures().push_back({expr, file, line, msg});
}

inline int runAllTests()
{
    int passed = 0;
    int failed = 0;
    auto &registry = getRegistry();

    std::cout << "========================================\n";
    std::cout << "Running " << registry.size() << " Domain Unit Tests\n";
    std::cout << "========================================\n";

    for (const auto &tc : registry)
    {
        getCurrentFailures().clear();
        try
        {
            tc.func();
        }
        catch (const std::exception &e)
        {
            recordFailure("Unhandled exception", __FILE__, __LINE__, e.what());
        }
        catch (...)
        {
            recordFailure("Unhandled non-std exception", __FILE__, __LINE__);
        }

        if (getCurrentFailures().empty())
        {
            std::cout << "  [PASS] " << tc.name << "\n";
            passed++;
        }
        else
        {
            std::cout << "  [FAIL] " << tc.name << "\n";
            for (const auto &f : getCurrentFailures())
            {
                std::cout << "         Assertion failed: (" << f.expression << ") at "
                          << f.file << ":" << f.line;
                if (!f.message.empty())
                {
                    std::cout << " - " << f.message;
                }
                std::cout << "\n";
            }
            failed++;
        }
    }

    std::cout << "========================================\n";
    std::cout << "Test Summary: " << passed << " passed, " << failed << " failed\n";
    std::cout << "========================================\n";

    return (failed == 0) ? 0 : 1;
}

} // namespace test

#define TEST_CASE(name)                                                              \
    static void name();                                                               \
    static const bool registered_##name = ::test::registerTest(#name, name);        \
    static void name()

#define ASSERT_TRUE(cond)                                                             \
    do {                                                                              \
        if (!(cond)) {                                                                \
            ::test::recordFailure(#cond, __FILE__, __LINE__);                         \
            return;                                                                   \
        }                                                                             \
    } while (0)

#define ASSERT_FALSE(cond)                                                            \
    do {                                                                              \
        if (cond) {                                                                   \
            ::test::recordFailure(#cond, __FILE__, __LINE__, "Expected false, got true"); \
            return;                                                                   \
        }                                                                             \
    } while (0)

#define ASSERT_EQ(val1, val2)                                                         \
    do {                                                                              \
        if (!((val1) == (val2))) {                                                    \
            ::test::recordFailure(#val1 " == " #val2, __FILE__, __LINE__);            \
            return;                                                                   \
        }                                                                             \
    } while (0)

#define ASSERT_NE(val1, val2)                                                         \
    do {                                                                              \
        if ((val1) == (val2)) {                                                       \
            ::test::recordFailure(#val1 " != " #val2, __FILE__, __LINE__);            \
            return;                                                                   \
        }                                                                             \
    } while (0)
