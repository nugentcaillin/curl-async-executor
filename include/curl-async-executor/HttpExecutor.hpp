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


class HttpResponseAwaitable
{
    bool await_ready();
    void await_suspend(std::coroutine_handle<> h);
    HttpResponse await_resume();
};

class HttpExecutor
{
public:
    HttpExecutor(int max_concurrent_requests, int num_threads);
    HttpResponseAwaitable await_async(HttpRequest request);
private:
    std::queue<HttpRequest> request_queue;
    std::mutex mu;
    CURLM* multi_handle;
    int easy_handle_count;
};
 

}

#endif // HTTP_EXECUTOR_H