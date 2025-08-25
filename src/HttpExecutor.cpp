#include "curl-async-executor/HttpExecutor.hpp"

namespace curl_async_executor
{

HttpExecutor::HttpResponseAwaitable HttpExecutor::await_async(HttpRequest request)
{
    return HttpResponseAwaitable(*this, std::move(request));
}

} // namespace curl_async_executor