#ifndef HTTP_EXECUTOR_H
#define HTTP_EXECUTOR_H

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <queue>
#include <mutex>
#include <curl/curl.h>
#include <coroutine>
#include <condition_variable>
#include <atomic>
#include <map>
#include <thread>

namespace curl_async_executor
{


class HttpExecutor
{
public:
    struct HttpResponseAwaitable;

    HttpExecutor(int max_concurrent_requests, int num_threads);
    ~HttpExecutor();
    HttpResponseAwaitable await_async(HttpRequest request);
private:
    int max_easy_handles_;
    std::atomic<int> easy_handle_count_;
    bool stop_requested_;
    std::queue<std::tuple<HttpRequest, std::coroutine_handle<>, HttpResponseAwaitable*>> request_queue_;
    std::mutex mu_;
    std::condition_variable cv_;
    void queue_request(std::coroutine_handle<> handle, HttpRequest request, HttpResponseAwaitable* awaitable);
    void worker_loop();
    std::thread worker_thread_;
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