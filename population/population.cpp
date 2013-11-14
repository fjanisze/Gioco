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

    //Search a family and remove them from the container.
    //Return a pointer to the removed family
    family_t* family_collection_t::remove_family( long family_id )
    {
        ELOG("family_collection_t::remove_family(): Requested to remove the family ID:", family_id);
        family_t* verdict = nullptr;
        family_container_iter iter = collection.find( family_id );
        if( iter != collection.end() )
        {
            verdict = iter->second;
            collection.erase( iter );
            return verdict;
        }
        return verdict;
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

    //Return the amount of families in the collection
    long family_collection_t::get_amount_of_families()
    {
        return collection.size();
    }

    //Set the residnece city for this family unit
    void family_collection_t::set_residence_city( long city_id )
    {
        residence_info.city_id = city_id;
    }

    //Set the building information.
    void family_collection_t::set_residence_building( long building_id )
    {
        residence_info.building_id = building_id;
    }

    //Return a copy of the residence information
    residence_info_t family_collection_t::get_residence_info()
    {
        return residence_info;
    }

    ////////////////////////////////////////////////////////////////////
    //
    //
    //	Follow the implementation for family_t , population_unit_t
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
            ++amount_of_members;
            return father->unit_id;
        }
        else if( status == family_status_t::is_mather )
        {
            mather = new_member( mather );
            ++amount_of_members;
            return mather->unit_id;
        }
        else
        {
            //Must be a kid, for now is not implemented!!
            LOG_WARN("family_t::add_member(): Trying to add a kid, NOT IMPLEMENTED NOW!!");
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

    ////////////////////////////////////////////////////////////////////
    //
    //
    //	Follow the implementation for population_manager
    //
    //
    ////////////////////////////////////////////////////////////////////

    population_unit_t::population_unit_t( long id ): gender( gender_t::male ) ,
                                            status( family_status_t::is_single ),
                                            age( NOT_VALID_AGE ),
                                            unit_id( id )
    {
    }

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

    //Return a pointer to the default collection (homeless)
    family_collection_t* population_manager::get_homeless_collection()
    {
        return family_collections.begin()->second;
    }

    //Return the ID for the homeless collection
    long population_manager::get_homeless_collection_id()
    {
        return family_collections.begin()->first; //This is the ID
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







