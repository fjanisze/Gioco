#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "../config.hpp"
#include "../logging/logger.hpp"
#include "gtest.h"

#ifdef COMPILE_TEST_MDOE





int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#endif
