
// #include "buffer_test.hpp"
// #include "socket_test.hpp"
// test files are compiled as separate translation units

#include <gtest/gtest.h>

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    
    return RUN_ALL_TESTS();
} 