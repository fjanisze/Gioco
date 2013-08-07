#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "population.hpp"

namespace population
{
    ////////////////////////////////////////////////////////////////////
    //
    //
    //	Follow the implementation for family_t , population_unit_t and population_manager
    //
    //
    ////////////////////////////////////////////////////////////////////

    long family_t::next_unit_id = 1;

    long family_t::get_next_id()
    {
        return ++next_unit_id;
    }

    family_t::family_t( long id ) : amount_of_members( 0 ) ,
                                    father( nullptr ),
                                    mather( nullptr ),
                                    kids( nullptr ),
                                    family_id( id )
    {
    }

    //Return the id of the new member, or NOT_VALID_ID if the operation fails.
    //The order of the members is defined as 1:father/single, 2:mather, n:kids..
    long family_t::add_member( short age, family_status_t status, gender_t gender )
    {
        //Add the member, if already exist just ovveride it
        auto new_member = [&]( population_unit_t* unit )
        {
            if( unit == nullptr )
            {
                unit = new(std::nothrow) population_unit_t( get_next_id () );
                assert( unit != nullptr );
            }
            unit->age = age;
            unit->status = status;
            unit->gender = gender;
            return unit;
        };

        if( status == family_status_t::is_single ||
            status == family_status_t::is_father )
        {
            father = new_member( father );
            return father->unit_id;
        }
        else if( status == family_status_t::is_mather )
        {
            mather = new_member( mather );
            return mather->unit_id;
        }
        else
        {
            //Must be a kid, for now is not implemented!!
            LOG_WARN("family_t::add_member(): Trying to add a kid, are not implemented now!!");
        }
        return NOT_VALID_ID;
    }

    long family_t::get_id()
    {
        return family_id;
    }

    population_unit_t::population_unit_t( long id ): gender( gender_t::male ) ,
                                            status( family_status_t::is_single ),
                                            age( NOT_VALID_AGE ),
                                            unit_id( id ),
                                            next( nullptr )
    {
    }

    long population_manager::next_family_id = 1;

    long population_manager::get_next_family_id()
    {
        return ++next_family_id;
    }

    //Create a new empty family object, the family is added immediatly to the internal container
    family_t* population_manager::create_family( )
    {
        family_t* family = new(std::nothrow) family_t( get_next_family_id() );
        assert( family != nullptr );
        family_container[ family->get_id() ] = family;
        return family;
    }

    //Look for a specific family and return a pointer to them
    family_t* population_manager::get_family( long id )
    {
        family_container_iter iter;
        if( ( iter = family_container.find( id ) ) != family_container.end() )
        {
            return iter->second;
        }
        return nullptr;
    }
}







