#include "gtest.h"
#include "..\population\population.hpp"
#include "..\config.hpp"

#if COMPILE_TEST_MODE

class PopulationTests : public ::testing::Test
{
protected:
    virtual void SetUp();
    virtual void TearDown();

    population::population_manager pop;
};

#endif
