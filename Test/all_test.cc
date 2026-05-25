
// #include "buffer_test.hpp"
// #include "socket_test.hpp"
// #include "poller_test.hpp"
#include "timerwheel_test.hpp"

#include <gtest/gtest.h>

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    
    return RUN_ALL_TESTS();
} 