#include "../config.hpp"
#include "../logging/logger.hpp"
#include "gtest.h"
#include "../events.hpp"

#if COMPILE_TEST_MODE

#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

int main(int argc, char **argv)
{
    log_inst.set_thread_name("GTEST_MAIN");

    ::testing::InitGoogleTest(&argc, argv);

    int verdict = RUN_ALL_TESTS();
    log_inst.terminate_logger();
    return verdict;
}

#endif
