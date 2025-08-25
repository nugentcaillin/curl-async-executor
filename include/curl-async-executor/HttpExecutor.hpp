#ifndef HTTP_EXECUTOR_H
#define HTTP_EXECUTOR_H

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <queue>
#include <mutex>
#include <curl/curl.h>
#include <coroutine>

namespace curl_async_executor
{


class HttpExecutor
{
public:
    struct HttpResponseAwaitable;

    // needs to be defined for unit tests of awaitable to work
    HttpExecutor(int max_concurrent_requests, int num_threads)
    {
        (void)max_concurrent_requests;
        (void)num_threads;
    }
    HttpResponseAwaitable await_async(HttpRequest request);
private:
    std::queue<HttpRequest> request_queue;
    std::mutex mu;
    CURLM* multi_handle;
    int easy_handle_count;
    void queue_request(std::coroutine_handle<> handle, HttpRequest request, HttpResponseAwaitable* awaitable);
};


// Awaitable that queues up requests inside HttpExecutor to be managed, returns HttpResponse created by value
struct HttpExecutor::HttpResponseAwaitable
{
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> handle)
    {
        executor_.queue_request(handle, std::move(request_), this);
    }
    HttpResponse await_resume()
    {
        return std::move(response_);
    }
    HttpResponseAwaitable(HttpExecutor& executor, HttpRequest req)
    : executor_(executor)
    , request_(std::move(req))
    {}
    HttpExecutor& executor_;
    HttpRequest request_;
    HttpResponse response_;
};



}

#endif // HTTP_EXECUTOR_H 