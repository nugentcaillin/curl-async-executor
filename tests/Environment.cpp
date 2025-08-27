#include <iostream>
#include <gtest/gtest.h>
#include <curl/curl.h>




class CurlEnvironment : public ::testing::Environment {
public:
    ~CurlEnvironment() override {};
    void SetUp() override {
        std::cout << "=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>in global setup\n";
        curl_global_init(CURL_GLOBAL_ALL);

    };
    void TearDown() override {
        std::cout << "=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>in global teardown\n";
        curl_global_cleanup();
    };
};

testing::Environment* const env = testing::AddGlobalTestEnvironment(new CurlEnvironment);

