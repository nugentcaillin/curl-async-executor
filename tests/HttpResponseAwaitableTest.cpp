#include <gtest/gtest.h>
#include <curl-async-executor/HttpExecutor.hpp>



using namespace curl_async_executor;

class HttpResponseAwaitableTest : public testing::Test
{

};


TEST_F(HttpResponseAwaitableTest, ReturnsHttpResponseDirectly)
{
    HttpRequest req = HttpRequestBuilder().build();
    HttpExecutor exec(1, 1);
    HttpExecutor::HttpResponseAwaitable awaitable(exec, std::move(req));
    awaitable.response_ = HttpResponse("test body", 0, 0);
    std::string body = awaitable.await_resume().get_body();
    EXPECT_EQ(body, "test body");
}

TEST_F(HttpResponseAwaitableTest, defaultBodyEmptyString)
{
    HttpRequest req = HttpRequestBuilder().build();
    HttpExecutor exec(1, 1);
    HttpExecutor::HttpResponseAwaitable awaitable(exec, std::move(req));

    std::string body = awaitable.await_resume().get_body();
    EXPECT_EQ(body, "");
}

TEST_F(HttpResponseAwaitableTest, awaitReadyFalse)
{
    HttpRequest req = HttpRequestBuilder().build();
    HttpExecutor exec(1, 1);
    HttpExecutor::HttpResponseAwaitable awaitable(exec, std::move(req));

    EXPECT_EQ(awaitable.await_ready(), false);
}