
#include <iostream>
#include <gtest/gtest.h>
#include <curl/curl.h>
#include <curl-async-executor/HttpExecutor.hpp>


using namespace curl_async_executor;

class HttpExecutorTest : public testing::Test
{
};



TEST_F(HttpExecutorTest, FreesMemoryCorrectly)
{
    HttpExecutor exec(1, 1);
}