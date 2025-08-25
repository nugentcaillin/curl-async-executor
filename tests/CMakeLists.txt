cmake_minimum_required(VERSION 3.15)
project(tests)

# google test has some old versions of CMake
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
  DOWNLOAD_EXTRACT_TIMESTAMP ON
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(curl_tests)

target_sources(curl_tests
    PRIVATE
        HttpRequestTest.cpp
        Environment.cpp
        HttpResponseAwaitableTest.cpp
        HttpResponseTest.cpp
)

target_link_libraries(curl_tests
    PRIVATE
        ProjectName
        GTest::gtest_main
)

CONFIGURE_TARGET(curl_tests)


include(GoogleTest)
gtest_discover_tests(curl_tests)