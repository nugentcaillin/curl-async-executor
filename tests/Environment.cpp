#include <iostream>
#include <gtest/gtest.h>




class CurlEnvironment : public ::testing::Environment {
public:
    ~CurlEnvironment() override {};
    void SetUp() override {
        std::cout << "=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>in global setup\n";
    };
    void TearDown() override {
        std::cout << "=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>=>in global teardown\n";
    };
};

testing::Environment* const env = testing::AddGlobalTestEnvironment(new CurlEnvironment);

