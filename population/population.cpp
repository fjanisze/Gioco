#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "population.hpp"

namespace population
{

    ////////////////////////////////////////////////////////////////////
    //
    //
    //	Follow the implementation for family_collection_t
    //
    //
    ////////////////////////////////////////////////////////////////////

    family_collection_t::family_collection_t( long id ) : collection_id( id )
    {
        ELOG("family_collection_t::family_collection_t(): Creating collection, ID:",id);
    }

    void family_collection_t::add_family( family_t* family )
    {
        ELOG("family_collection_t::add_family(): Adding the family ID:",family->get_id(), ", to collection ID:",collection_id );
        collection[ family->get_id() ] = family;
    }

    //Return the amount of unit which are part of the collection
    long family_collection_t::get_population()
    {
        long sum = 0;
        for( auto& elem : collection )
        {
            sum += elem.second->get_amount_of_members();
        }
        return sum;
    }

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
                                    family_id( id )
    {
    }

    short family_t::get_amount_of_members()
    {
        return amount_of_members;
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

    family_t::~family_t()
    {
        if( father )
        {
            delete father;
        }
        if( mather )
        {
            delete mather;
        }
    }

    population_unit_t::population_unit_t( long id ): gender( gender_t::male ) ,
                                            status( family_status_t::is_single ),
                                            age( NOT_VALID_AGE ),
                                            unit_id( id )
    {
    }

    long population_manager::next_family_id = 1;
    long population_manager::next_collection_id = 1;

    long population_manager::get_next_family_id()
    {
        return ++next_family_id;
    }

    long population_manager::get_next_coll_id()
    {
        return ++next_collection_id;
    }

    //Create a new empty family object, the family is added immediatly to the internal container
    family_t* population_manager::create_family( )
    {
        family_t* family = new(std::nothrow) family_t( get_next_family_id() );
        assert( family != nullptr );
        family_container[ family->get_id() ] = family;
        ELOG("population_manager::create_family(): New family created, ID:",family->get_id() );
        return family;
    }

    long population_manager::create_collection()
    {
        long id = get_next_coll_id();
        ELOG("population_manager::create_collection(): Creating a new collection, ID:",id);
        family_collection_t* new_collection = new(std::nothrow) family_collection_t( id );
        assert( new_collection != nullptr );
        family_collections[ id ] = new_collection;
        return id;
    }

    //Return the pointe to the family_collection_t element or nullptr
    family_collection_t* population_manager::get_collection( long id )
    {
        family_coll_iter iter;
        if( ( iter = family_collections.find( id ) ) != family_collections.end() )
        {
            return iter->second;
        }
        return nullptr;
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

    population_manager::population_manager()
    {
        LOG("population_manager::population_manager(): Creating the population manager!");
        //Create the default collection for the homeless family
        homeless_collection_id = create_collection();
        LOG("population_manager::population_manager(): Homeless collection ID:", homeless_collection_id );
    }

    //Clear all the used memory
    population_manager::~population_manager()
    {
        for( auto& elem : family_container )
        {
            delete elem.second;
        }
        family_container.clear();
    }
}







