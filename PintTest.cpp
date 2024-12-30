#include "PintTest.h"

#include <vector>
#include <string_view>
#include <iostream>

namespace
{
    int times2(int x) // function under test
    {
        return x * 2;
    }
    TEST(testtimes2)
    {
        EXPECT_EQ(4, times2(2));
        ASSERT_EQ(6, times2(3));
        for (int i = 0; i < 3; ++i)
            ASSERT_EQ(i * 2, times2(i)) << i;
    }
    TEST(testtimes2Wrong)
    {
        ASSERT_NE(7, times2(3));
        EXPECT_NE(5, times2(2));
        for (int i = 1; i < 3; ++i)
            ASSERT_NE(0, times2(i)) << i;
    }
    TEST(ThisAlwaysFails)
    {
        EXPECT_TRUE(false);
    }
}

int main()
{
    {
        const auto rc = PintTest::runAllTests(0, nullptr);
        if (0 == rc)
        {
            std::cerr << "Test failed at line " << __LINE__ << "\n";
            return 1; // it should always fail because of the failing test
        }
    }
    {
        const char* argv[]{ "p1", "p2" };
        const auto rc = PintTest::runAllTests(2, argv);
        if (0 == rc)
        {
            std::cerr << "Test failed at line " << __LINE__ << "\n";
            return 1; // it should always fail because of the failing test
        }
    }
    {
        const char* argv[]{ "--filter=-ThisAlwaysFails" };
        const auto rc = PintTest::runAllTests(1, argv);
        if (0 != rc)
        {
            std::cerr << "Test failed at line " << __LINE__ << "\n";
            return 1;
        }
    }
    {
        const auto [ran, failed] = PintTest::runAllTests2(std::vector<std::string_view>{});
        if (1 != failed)
        {
            std::cerr << "Test failed at line " << __LINE__ << "\n";
            return 1;
        }
        if (ran == 0)
        {
            std::cerr << "Test failed at line " << __LINE__ << "\n";
            return 1;
        }
        if (ran != 4)
        {
            std::cerr << "Test failed at line " << __LINE__ << "\n";
            return 1;
        }
    }
    {
        const auto [ran, failed] = PintTest::runAllTests2(std::vector<std::string_view>{"abc"});
        if (1 != failed)
        {
            std::cerr << "Test failed at line " << __LINE__ << "\n";
            return 1;
        }
        if (ran == 0)
        {
            std::cerr << "Test failed at line " << __LINE__ << "\n";
            return 1;
        }
        if (ran != 4)
        {
            std::cerr << "Test failed at line " << __LINE__ << "\n";
            return 1;
        }
    }
    {
        for (const auto& vec :
            {
                std::vector<std::string_view>{"abc"}
                ,std::vector<std::string_view>{"--filter"}
            })
        {
            const auto [ran, failed] = PintTest::runAllTests2(vec);
            if (1 != failed)
            {
                std::cerr << "Test failed at line " << __LINE__ << "\n";
                return 1;
            }
            if (ran != 4)
            {
                std::cerr << "Test failed at line " << __LINE__ << "\n";
                return 1;
            }
        }
    }
    for (const auto& vec :
        {
            std::vector<std::string_view>{"--filter="}
            ,std::vector<std::string_view>{"--filter=abc"}
        })
    {
        const auto [ran, failed] = PintTest::runAllTests2(vec);
        if (0 != failed)
        {
            std::cerr << "Test failed at line " << __LINE__ << "\n";
            return 1;
        }
        if (ran != 1)
        {
            std::cerr << "Test failed at line " << __LINE__ << "\n";
            return 1;
        }
    }
    {
        const auto [ran, failed] = PintTest::runAllTests2(std::vector<std::string_view>{"--filter=abc", "--filter=def"});
        if (-1 != ran)
        {
            std::cerr << "Test failed at line " << __LINE__ << "\n";
            return 1;
        }
    }
    {
        const auto [ran, failed] = PintTest::runAllTests2(std::vector<std::string_view>{"--filter=abc", "--filter=-def"});
        if (-1 != ran)
        {
            std::cerr << "Test failed at line " << __LINE__ << "\n";
            return 1;
        }
    }
    for (const auto& vec :
        {
            std::vector<std::string_view>{"--filter=rong"}
            ,std::vector<std::string_view>{"--filter=Wrong"}
        })
    {
        const auto [ran, failed] = PintTest::runAllTests2(vec);
        if (0 != failed)
        {
            std::cerr << "Test failed at line " << __LINE__ << "\n";
            return 1;
        }
        if (ran == 0)
        {
            std::cerr << "Test failed at line " << __LINE__ << "\n";
            return 1;
        }
        if (ran != 2)
        {
            std::cerr << "Test failed at line " << __LINE__ << "\n";
            return 1;
        }
    }
    for (const auto& vec :
        {
            std::vector<std::string_view>{"--filter=-ThisAlwaysFails"}
            ,std::vector<std::string_view>{"--filter=-Fails"}
        })
    {
        const auto [ran, failed] = PintTest::runAllTests2(vec);
        if (0 != failed)
        {
            std::cerr << "Test failed at line " << __LINE__ << "\n";
            return 1;
        }
        if (ran != 3)
        {
            std::cerr << "Test failed at line " << __LINE__ << "\n";
            return 1;
        }
    }
    return 0;
}
