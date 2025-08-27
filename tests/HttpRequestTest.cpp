#include <gtest/gtest.h>
#include <curl-async-executor/HttpRequest.hpp>
#include <curl/curl.h>
#include <iostream>


using namespace curl_async_executor;

class HttpRequestTest : public testing::Test
{
public:
    curl_slist* header_helper(const HttpRequest& req)
    {
        return req.headers_;
    }
    CURL* handle_helper(const HttpRequest& req)
    {
        return req.handle_;
    }
    const std::string& body_helper(const HttpRequest& req)
    {
        return req.body_;
    }
};

TEST_F(HttpRequestTest, headersNotNullAfterAddHeader)
{
    HttpRequest req = HttpRequestBuilder()
                        .add_header("Foo", "bar")
                        .build();
    EXPECT_NE(header_helper(req), nullptr);
}
TEST_F(HttpRequestTest, bodyNotNullAfterSetting)
{
    HttpRequest req = HttpRequestBuilder()
                        .set_body("Test body!")
                        .build();
    
    EXPECT_EQ(body_helper(req), "Test body!");
}


TEST_F(HttpRequestTest, setsUrlCorrectly)
{
    HttpRequest req = HttpRequestBuilder()
                        .set_url("https://localhost/dummy") 
                        .build();
    char *url {nullptr};
    curl_easy_getinfo(handle_helper(req), CURLINFO_EFFECTIVE_URL, &url);
    ASSERT_NE(url, nullptr);
    std::string set_url(url);
    EXPECT_EQ(set_url, "https://localhost/dummy");
}

// test passes if no leaks detected
TEST_F(HttpRequestTest, requestFreesCorrectlyWithHeaders)
{
    HttpRequest req = HttpRequestBuilder()
                        .add_header("Foo", "bar")
                        .build();
}

// test passes if no leaks detected
TEST_F(HttpRequestTest, nonConstructedRequestFreesCorrectly)
{
    HttpRequestBuilder builder = HttpRequestBuilder(); 
    builder.add_header("foo", "bar");
}


// test passes if no leaks detected
TEST_F(HttpRequestTest, requestFreesCorrectlyWithoutHeadersOrBody)
{
    HttpRequest req = HttpRequestBuilder()
                        .build();
}


// test passes if no leaks detected
TEST_F(HttpRequestTest, requestFreesCorrectlyWithBody)
{
    HttpRequest req = HttpRequestBuilder()
                        .set_body("Test body!")
                        .build();
}


TEST_F(HttpRequestTest, moveSemanticsCorrectWithHeader)
{
    HttpRequest req1 = HttpRequestBuilder()
                        .add_header("foo", "bar")
                        .build();
    HttpRequest req2 = std::move(req1);
    EXPECT_EQ(handle_helper(req1), nullptr);
    EXPECT_EQ(header_helper(req1), nullptr);
    EXPECT_NE(handle_helper(req2), nullptr);
    EXPECT_NE(header_helper(req2), nullptr);
}


TEST_F(HttpRequestTest, moveSemanticsCorrectWithoutHeader)
{
    HttpRequest req1 = HttpRequestBuilder()
                        .build();
    HttpRequest req2 = std::move(req1);
    EXPECT_EQ(handle_helper(req1), nullptr);
    EXPECT_EQ(header_helper(req1), nullptr);
    EXPECT_NE(handle_helper(req2), nullptr);
    EXPECT_EQ(header_helper(req2), nullptr);
}
