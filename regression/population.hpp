#if COMPILE_TEST_MODE

#include "gtest.h"
#include "..\population\population.hpp"

class PopulationTests : public ::testing::Test
{
protected:
    virtual void SetUp();
    virtual void TearDown();

    population::population_manager pop;
};

#endif
