#include "curl-async-executor/HttpExecutor.hpp"
#include <stdexcept>
#include <coroutine>

namespace curl_async_executor
{


HttpExecutor::HttpExecutor(int max_concurrent_requests, int num_threads)
: max_easy_handles_(max_concurrent_requests)
, easy_handle_count_(0) 
, stop_requested_(false)
{
    (void)num_threads;
}

HttpExecutor::HttpResponseAwaitable HttpExecutor::await_async(HttpRequest request)
{
    return HttpResponseAwaitable(*this, std::move(request));
}


void HttpExecutor::queue_request(std::coroutine_handle<> handle, HttpRequest request, HttpResponseAwaitable* awaitable)
{
    (void)handle;
    (void)request;
    (void)awaitable;

}

void HttpExecutor::worker_loop()
{
    CURLMcode status_m {};
    //CURLcode status_e {};
    CURLM* multi = curl_multi_init();
    int thread_request_count { 0 };

    // HttpRequest is move only, and we want to process outside of lock so we keep 
    // a dummy HttpRequest to move into and a flag to signify it needs to be processed
    bool request_captured { false };
    HttpRequest request = HttpRequestBuilder().build();
    std::coroutine_handle<> continuation_handle;
    HttpResponseAwaitable* awaitable;
    std::map<CURL*, std::tuple<std::coroutine_handle<>, HttpRequest, HttpResponseAwaitable*>> inflight_requests_;

    while (true)
    {
        // get reccomended duration to wait from curl, and change to timepoint
        long suggested_timeout;
        status_m = curl_multi_timeout(multi, &suggested_timeout);
        if (status_m != CURLM_OK) throw std::runtime_error("curl_multi_timeout did not return CURLE_OK");
        std::chrono::time_point wait_until { std::chrono::system_clock::now() + std::chrono::milliseconds(suggested_timeout) };
        
        // wait until stop requested, request queued, or if this thread is processing a request and we
        // have reached curl's suggested timeout
        {
            std::unique_lock<std::mutex> lk(mu_);
            cv_.wait(lk, [this, wait_until, thread_request_count]()
            { 
                return stop_requested_
                    || (thread_request_count > 0 && std::chrono::system_clock::now() > wait_until)
                    || (request_queue_.size() > 0 && easy_handle_count_.load() < max_easy_handles_);
            });

            // only exit loop when queue and current requests dealt with
            if (stop_requested_ && thread_request_count == 0 && request_queue_.size() == 0) break;

            // capture one request from queue if exists and move it to be processed outside of lock
            if (request_queue_.size() > 0 && easy_handle_count_.load() < max_easy_handles_)
            {
                easy_handle_count_.fetch_add(1);
                thread_request_count++;
                request_captured = true;
                request = std::move(std::get<0>(request_queue_.front()));
                continuation_handle = std::get<1>(request_queue_.front());
                awaitable = std::get<2>(request_queue_.front());
                request_queue_.pop();
            }

        }

        // enrol request
        if (request_captured)
        {
            status_m = curl_multi_add_handle(multi, request.get_handle());
            if (status_m != CURLM_OK) throw std::runtime_error("Error adding curl_easy_handle to multi_handle");

            std::tuple<std::coroutine_handle<>, HttpRequest, HttpResponseAwaitable*> inflight_request { continuation_handle, std::move(request), awaitable };
            inflight_requests_.emplace(request.get_handle(), std::move(inflight_request));
        }

        // deal with finished requests
        if (thread_request_count > 0 && std::chrono::system_clock::now() > wait_until)
        {
            int msg_count;
            curl_multi_perform(multi, &msg_count);
            struct CURLMsg *m;
            do {

                m = curl_multi_info_read(multi, &msg_count);
                long int http_status {};
                int curl_status { m->msg };
                if (m && (m->msg == CURLMSG_DONE))
                {
                    // extract data from request, construct response, give to awaitable and resume coroutine
                    CURL *easy_handle = m->easy_handle;
                    auto it = inflight_requests_.find(easy_handle);
                    curl_easy_getinfo(easy_handle, CURLINFO_RESPONSE_CODE, &http_status);
                    
                    HttpRequest finished_request = std::move(std::get<1>(it->second));
                    std::coroutine_handle<> finished_continuation = std::get<0>(it->second);
                    HttpResponseAwaitable* finished_awaitable = std::get<2>(it->second);

                    HttpResponse res(finished_request.get_body_data(), http_status, curl_status);

                    finished_awaitable->response_ = std::move(res);
                    finished_continuation.resume();
                }
            } while (m);
        }

        request_captured = false;
    }
}


} // namespace curl_async_executor