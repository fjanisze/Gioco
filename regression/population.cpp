#if COMPILE_TEST_MODE

#include "population.hpp"

void PopulationTests::SetUp()
{
}

void PopulationTests::TearDown()
{
}

//Regression for population_manager
TEST_F( PopulationTests , CollectionBasicOperation )
{
    //The first collection is create automatically, should be the one for the homeless
    long homeless_collection_id = pop.get_homeless_collection_id();
    ASSERT_NE( nullptr , pop.get_collection( homeless_collection_id ) );
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

//Family creation tests
TEST_F( PopulationTests , FamilyBasicOperation )
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
    ASSERT_EQ( 2 , family.get_amount_of_members() );
}

//Adding family elements to a population collection
TEST_F( PopulationTests , AddingRemovingHomelessFamilies )
{
    //Create some families
    population::family_t family1( 1 ) , family2( 2 ), family3( 3 );

    family1.add_member( 20 , population::family_status_t::is_father , population::gender_t::male );
    family1.add_member( 25 , population::family_status_t::is_mather , population::gender_t::female );

    family2.add_member( 31 , population::family_status_t::is_father , population::gender_t::male );
    family2.add_member( 33 , population::family_status_t::is_mather , population::gender_t::female );

    family3.add_member( 44 , population::family_status_t::is_single , population::gender_t::male );

    //Get the homeless collection
    population::family_collection_t* coll_ptr = pop.get_homeless_collection();
    ASSERT_NE( nullptr , coll_ptr );
    //Add the families
    coll_ptr->add_family( &family1 );
    coll_ptr->add_family( &family2 );
    coll_ptr->add_family( &family3 );

    //Check if the amount of person match
    ASSERT_EQ( 5 , coll_ptr->get_population() );
    //Check the amount of families
    ASSERT_EQ( 3 , coll_ptr->get_amount_of_families() );

    //Now remove the families
    population::family_t* family1_ptr = coll_ptr->remove_family( family1.get_id() );
    population::family_t* family2_ptr = coll_ptr->remove_family( family2.get_id() );
    ASSERT_NE( nullptr , family1_ptr );
    ASSERT_NE( nullptr , family2_ptr );

    //Check if proper families were removed
    ASSERT_EQ( &family1 , family1_ptr );
    ASSERT_EQ( &family2 , family2_ptr );

    //Check collection size
    ASSERT_EQ( 1 , coll_ptr->get_population() );
    ASSERT_EQ( 1 , coll_ptr->get_amount_of_families() );
}


#endif












