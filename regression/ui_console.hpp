#include "..\config.hpp"

#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "gtest.h"
#include "..\ui\ui_console.hpp"

#if COMPILE_TEST_MODE

class UiConsoleTests : public ::testing::Test
{
protected:
    UiConsoleTests();
    ~UiConsoleTests();

    virtual void SetUp();
    virtual void TearDown();

    graphic_ui::console_manager console;
};

#endif
