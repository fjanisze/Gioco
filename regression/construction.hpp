#include "gtest.h"
#include "..\config.hpp"
#include "..\game.hpp"

#if COMPILE_TEST_MODE

class ConstructionTests : public ::testing::Test
{
protected:
    ConstructionTests();
    ~ConstructionTests();

    virtual void SetUp();
    virtual void TearDown();

    game_manager::game_manager* game;
    cities::city_agent* roma_agent;
};

#endif
