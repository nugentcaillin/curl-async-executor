#include <gtest/gtest.h>
#include <curl-async-executor/HttpResponse.hpp>


using namespace curl_async_executor;

class HttpResponseTest : public testing::Test
{

};

TEST_F(HttpResponseTest, defaultsCorrect)
{
    HttpResponse res;
    EXPECT_EQ(res.get_body(), "");
    EXPECT_EQ(res.get_http_status(), 0);
    EXPECT_EQ(res.get_curl_status(), 0);
}

TEST_F(HttpResponseTest, gettersCorrect)
{
    HttpResponse res("foo", 1, 2);
    EXPECT_EQ(res.get_body(), "foo");
    EXPECT_EQ(res.get_http_status(), 1);
    EXPECT_EQ(res.get_curl_status(), 2);
}

TEST_F(HttpResponseTest, moveSemanticsCorrect)
{
    HttpResponse res("foo", 1, 2);
    HttpResponse res2 = std::move(res);

    EXPECT_EQ(res.get_body(), "");
    EXPECT_EQ(res.get_http_status(), 0);
    EXPECT_EQ(res.get_curl_status(), 0);
    EXPECT_EQ(res2.get_body(), "foo");
    EXPECT_EQ(res2.get_http_status(), 1);
    EXPECT_EQ(res2.get_curl_status(), 2);

    HttpResponse res3 { std::move(res2) };

    EXPECT_EQ(res2.get_body(), "");
    EXPECT_EQ(res2.get_http_status(), 0);
    EXPECT_EQ(res2.get_curl_status(), 0);
    EXPECT_EQ(res3.get_body(), "foo");
    EXPECT_EQ(res3.get_http_status(), 1);
    EXPECT_EQ(res3.get_curl_status(), 2);

    HttpResponse res4("bar", 3, 4);

    res4 = std::move(res3);

    EXPECT_EQ(res3.get_body(), "");
    EXPECT_EQ(res3.get_http_status(), 0);
    EXPECT_EQ(res3.get_curl_status(), 0);
    EXPECT_EQ(res4.get_body(), "foo");
    EXPECT_EQ(res4.get_http_status(), 1);
    EXPECT_EQ(res4.get_curl_status(), 2);
}
