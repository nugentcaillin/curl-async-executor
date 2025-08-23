#include <test/test.hpp>
#include <gtest/gtest.h>


TEST(BasicTest, testAddOne)
{
    EXPECT_EQ(test::return_one(), 1);
}

