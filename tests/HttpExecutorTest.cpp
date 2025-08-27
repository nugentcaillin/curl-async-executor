
#include <iostream>
#include <gtest/gtest.h>
#include <curl/curl.h>
#include <curl-async-executor/HttpExecutor.hpp>
#include <coroutine>
#include <future>


/*
These tests rely on a http server being set up such that GET http://localhost:8080
is a valid http request and GET http://localhost:8081 is not.
run_tests.sh and the github workflow sets this up with ncat
*/

using namespace curl_async_executor;

class HttpExecutorTest : public testing::Test
{
};

struct HttpExecutorTestCoro
{
    struct promise_type
    {
        using handle = std::coroutine_handle<promise_type>; 
        std::suspend_never initial_suspend() { return {}; };
        std::suspend_always final_suspend() noexcept { return {}; }
        HttpExecutorTestCoro get_return_object() { return HttpExecutorTestCoro(handle::from_promise(*this)); }
        void return_void() {};
        void unhandled_exception() {};
    };

    HttpExecutorTestCoro(promise_type::handle handle)
    : handle_(handle)
    {}

    HttpExecutorTestCoro(const HttpExecutorTestCoro& other) = delete;
    HttpExecutorTestCoro& operator=(const HttpExecutorTestCoro& other) = delete;
    HttpExecutorTestCoro(HttpExecutorTestCoro&& other)
    :handle_(nullptr)
    {
        std::swap(handle_, other.handle_);
    }
    HttpExecutorTestCoro& operator=(HttpExecutorTestCoro&& other)
    {
        if (this == &other) return *this;
        if (handle_) handle_.destroy();
        handle_ = nullptr;
        std::swap(handle_, other.handle_);
        return *this;
    }

    promise_type::handle handle_;
    ~HttpExecutorTestCoro()
    {
        if (handle_) handle_.destroy();
    }
    
};

HttpExecutorTestCoro simple_request(HttpExecutor& exec, std::promise<HttpResponse> signal_completion, std::string url)
{
    HttpRequest req = HttpRequestBuilder().set_url(url).build();
    HttpResponse res = co_await exec.await_async(std::move(req));
    signal_completion.set_value(std::move(res));
}


HttpExecutorTestCoro complex_request(HttpExecutor& exec, std::promise<bool> signal_correct_completion, std::string url)
{
    bool all_correct = true;
    HttpRequest req = HttpRequestBuilder().set_url(url).build();
    HttpResponse res = co_await exec.await_async(std::move(req));
    if (res.get_body().length() < 1 || res.get_http_status() != 200 || res.get_curl_status() != CURLE_OK) all_correct = false;


    req = HttpRequestBuilder().set_url(url).build();
    res = co_await exec.await_async(std::move(req));
    if (res.get_body().length() < 1 || res.get_http_status() != 200 || res.get_curl_status() != CURLE_OK) all_correct = false;

    req = HttpRequestBuilder().set_url(url).build();
    res = co_await exec.await_async(std::move(req));
    if (res.get_body().length() < 1 || res.get_http_status() != 200 || res.get_curl_status() != CURLE_OK) all_correct = false;

    signal_correct_completion.set_value(all_correct);
}

HttpExecutorTestCoro complex_request_only_completion(HttpExecutor& exec, std::promise<void> signal_correct_completion, std::string url)
{
    HttpRequest req = HttpRequestBuilder().set_url(url).build();
    HttpResponse res = co_await exec.await_async(std::move(req));
    req = HttpRequestBuilder().set_url(url).build();
    res = co_await exec.await_async(std::move(req));
    req = HttpRequestBuilder().set_url(url).build();
    res = co_await exec.await_async(std::move(req));

    signal_correct_completion.set_value();
}


TEST_F(HttpExecutorTest, FreesMemoryCorrectly)
{
    HttpExecutor exec(1, 1);
}

TEST_F(HttpExecutorTest, validRequestHasNonNullBody)
{
    std::promise<HttpResponse> completion_promise {};
    std::future<HttpResponse> completion_future { completion_promise.get_future() };
    HttpExecutor exec(1, 1);
    auto coro = simple_request(exec, std::move(completion_promise), "http://localhost:8080/");

    completion_future.wait_for(std::chrono::seconds(5));

    HttpResponse res = completion_future.get();
    EXPECT_TRUE(res.get_body().length() > 1);
    EXPECT_EQ(res.get_http_status(), 200);
    EXPECT_EQ(res.get_curl_status(), CURLE_OK);
}

// future tests:

// invalid curl request
TEST_F(HttpExecutorTest, invalidRequestReturnsError)
{
    std::promise<HttpResponse> completion_promise {};
    std::future<HttpResponse> completion_future { completion_promise.get_future() };
    HttpExecutor exec(1, 1);
    auto coro = simple_request(exec, std::move(completion_promise), "http://localhost:8081");

    completion_future.wait_for(std::chrono::seconds(5));

    HttpResponse res = completion_future.get();
    EXPECT_NE(res.get_http_status(), 200);
    EXPECT_NE(res.get_curl_status(), CURLE_OK);
}

// multiple requests
TEST_F(HttpExecutorTest, multipleRequestsWorkCorrectlyOneAtATime)
{
    HttpExecutor exec(1, 1);
    std::promise<HttpResponse> completion_promise1 {};
    std::future<HttpResponse> completion_future1 { completion_promise1.get_future() };
    auto coro1 = simple_request(exec, std::move(completion_promise1), "http://localhost:8081");

    std::promise<HttpResponse> completion_promise2 {};
    std::future<HttpResponse> completion_future2 { completion_promise2.get_future() };
    auto coro2 = simple_request(exec, std::move(completion_promise2), "http://localhost:8080/");

    std::promise<HttpResponse> completion_promise3 {};
    std::future<HttpResponse> completion_future3 { completion_promise3.get_future() };
    auto coro3 = simple_request(exec, std::move(completion_promise3), "http://localhost:8080/");

    

    completion_future1.wait_for(std::chrono::seconds(5));
    completion_future2.wait_for(std::chrono::seconds(5));
    completion_future3.wait_for(std::chrono::seconds(5));

    HttpResponse res = completion_future1.get();
    EXPECT_NE(res.get_http_status(), 200);
    EXPECT_NE(res.get_curl_status(), CURLE_OK);

    res = completion_future2.get();
    EXPECT_EQ(res.get_http_status(), 200);
    EXPECT_EQ(res.get_curl_status(), CURLE_OK);
    EXPECT_TRUE(res.get_body().length() > 1);

    res = completion_future3.get();
    EXPECT_EQ(res.get_http_status(), 200);
    EXPECT_EQ(res.get_curl_status(), CURLE_OK);
    EXPECT_TRUE(res.get_body().length() > 1);

}


TEST_F(HttpExecutorTest, multipleRequestsWorkCorrectlyConcurrently)
{
    HttpExecutor exec(10, 1);
    std::promise<HttpResponse> completion_promise1 {};
    std::future<HttpResponse> completion_future1 { completion_promise1.get_future() };
    auto coro1 = simple_request(exec, std::move(completion_promise1), "http://localhost:8081");

    std::promise<HttpResponse> completion_promise2 {};
    std::future<HttpResponse> completion_future2 { completion_promise2.get_future() };
    auto coro2 = simple_request(exec, std::move(completion_promise2), "http://localhost:8080/");

    std::promise<HttpResponse> completion_promise3 {};
    std::future<HttpResponse> completion_future3 { completion_promise3.get_future() };
    auto coro3 = simple_request(exec, std::move(completion_promise3), "http://localhost:8080/");

    

    completion_future1.wait_for(std::chrono::seconds(5));
    completion_future2.wait_for(std::chrono::seconds(5));
    completion_future3.wait_for(std::chrono::seconds(5));

    HttpResponse res = completion_future1.get();
    EXPECT_NE(res.get_http_status(), 200);
    EXPECT_NE(res.get_curl_status(), CURLE_OK);

    res = completion_future2.get();
    EXPECT_EQ(res.get_http_status(), 200);
    EXPECT_EQ(res.get_curl_status(), CURLE_OK);
    EXPECT_TRUE(res.get_body().length() > 1);

    res = completion_future3.get();
    EXPECT_EQ(res.get_http_status(), 200);
    EXPECT_EQ(res.get_curl_status(), CURLE_OK);
    EXPECT_TRUE(res.get_body().length() > 1);

}



// multiple coroutines performing multiple requests works correctly


TEST_F(HttpExecutorTest, multipleCoroutinesAwaitingMultipleRequests)
{
    HttpExecutor exec(1, 1);
    std::promise<bool> completion_promise1 {};
    std::future<bool> completion_future1 { completion_promise1.get_future() };
    auto coro1 = complex_request(exec, std::move(completion_promise1), "http://localhost:8081");

    std::promise<bool> completion_promise2 {};
    std::future<bool> completion_future2 { completion_promise2.get_future() };
    auto coro2 = complex_request(exec, std::move(completion_promise2), "http://localhost:8080/");

    std::promise<bool> completion_promise3 {};
    std::future<bool> completion_future3 { completion_promise3.get_future() };
    auto coro3 = complex_request(exec, std::move(completion_promise3), "http://localhost:8080/");

    

    completion_future1.wait_for(std::chrono::seconds(5));
    completion_future2.wait_for(std::chrono::seconds(5));
    completion_future3.wait_for(std::chrono::seconds(5));

    bool res = completion_future1.get();
    EXPECT_EQ(res, false);

    res = completion_future2.get();
    EXPECT_EQ(res, true);

    res = completion_future3.get();
    EXPECT_EQ(res, true);

}

// stress test with multiple coroutines queueing multiple requests doesn't cause leaks

TEST_F(HttpExecutorTest, manyCoroutinesWithMultipleRequests)
{
    int num_complex_coroutines { 1000 };
    HttpExecutor exec(100, 1);
    std::vector<std::future<void>> completion_futures;
    std::vector<HttpExecutorTestCoro> coros;
    for (int i { 0 }; i < num_complex_coroutines; ++i)
    {
        std::promise<void> completion_promise;
        completion_futures.emplace_back(completion_promise.get_future());
        // we only test completion since some of these requests will overwhelm netcat
        coros.emplace_back(complex_request_only_completion(exec, std::move(completion_promise), "http://localhost:8080/"));
    }
    auto wait_until_time = std::chrono::system_clock::now() + std::chrono::seconds(20);
    for (int i {0}; i < num_complex_coroutines; ++i)
    {
        auto status = completion_futures[i].wait_until(wait_until_time);
        EXPECT_EQ(status, std::future_status::ready);
    }
}