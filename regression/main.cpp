#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "../config.hpp"
#include "../logging/logger.hpp"
#include "gtest.h"

#ifdef COMPILE_TEST_MDOE

#include "..\population\population.hpp"

//Family creation tests
TEST( FamilyTypeTest , FamilyBasicOperation )
{
    //Create a family
    population::family_t family( 1 );
    ASSERT_EQ( 1 , family.get_id() );
    ASSERT_EQ( 0 , family.get_amount_of_members() );
    //Add some members
    long id = family.add_member( 20 , population::family_status_t::is_father , population::gender_t::male );
    ASSERT_EQ( 2 , id );
    id = family.add_member( 30 , population::family_status_t::is_mather , population::gender_t::female );
    ASSERT_EQ( 3 , id );
}

//Regression for population_manager
TEST( CollectionTest , CollectionBasicOperation )
{
    population::population_manager pop;
    //The first collection is create automatically, should be the one for the homeless
    ASSERT_NE( nullptr , pop.get_collection( 2 ) );
    //Create a couple of collections
    long id_1 = pop.create_collection();
    ASSERT_EQ( 3 , id_1 );
    long id_2 = pop.create_collection();
    ASSERT_EQ( 4 , id_2 );
    //Now tries to get the collection
    population::family_collection_t* coll_ptr = pop.get_collection( id_1 );
    ASSERT_NE( nullptr , coll_ptr );
    coll_ptr = pop.get_collection( id_2 );
    ASSERT_NE( nullptr , coll_ptr );
}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#endif
