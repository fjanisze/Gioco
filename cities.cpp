#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "cities.hpp"

namespace cities
{
    ////////////////////////////////////////////////////////////////////
    //
    //
    //	Follow the implementation for city_manager, city_agent and city_info_t
    //
    //
    ////////////////////////////////////////////////////////////////////

    long city_manager::next_city_id = 1;

    city_info_t::city_info_t( long id , const std::string& city_name  ) : city_id( id ) , name( city_name )
    {
        ELOG("city_info_t::city_info_t(): Creating a new city_info_t with ID: ", city_id, ", city name: ", name );
        citymap = nullptr;
    }

    //Constructor for city_manager
    city_manager::city_manager( game_map::map_viewport_settings_t viewport ) : viewport_settings( viewport )
    {
        LOG("city_manager::city_manager(): Creating the object.");
    }

    //Return the next unique city identifier
    long city_manager::get_next_city_id()
    {
        return next_city_id++;
    }

    //Create a new city
    city_agent* city_manager::create_new_city( const std::string& name , long size )
    {
        long city_id = get_next_city_id();
        LOG("city_manager::create_new_city(): Creating a new city, name: ", name , ", ID: ", city_id , ", size: ", size );
        city_info_t* new_city = new(std::nothrow) city_info_t( city_id , name );
        assert( new_city != nullptr );
        //Going ahead with the resource creation
        new_city->citymap = new(std::nothrow) citymap_t( size, viewport_settings );
        assert( new_city->citymap != nullptr );
        //Add to the container
        city_container.push_back( new_city );
        //Create also the city agent
        city_agent* agent = create_new_agent( new_city );
        assert( agent != nullptr ); //Should not be null..
        return agent;
    }

    //The function also add the city agent in the respective structure
    city_agent* city_manager::create_new_agent( city_info_t* city_info )
    {
        LOG("city_manager::create_new_agent(): New agent for ", city_info->name );
        //Check if the agent for this city already exist
        for( auto elem : agents )
        {
            if( elem->get_city_id() == city_info->city_id )
            {
                LOG_WARN("city_manager::create_new_agent(): The agent for this city already exist, no need to create a new one.");
                return nullptr; //The agent already exist
            }
        }
        //Create a new agent
        city_agent* agent = new(std::nothrow) city_agent( this , city_info );
        assert( agent != nullptr );
        agents.push_back( agent );
        return agent;
    }

    //Return the pointer to the city_info_t
    city_info_t* city_manager::find_city_info( long city_id )
    {
        city_info_t* city = nullptr;
        for( auto elem : city_container )
        {
            if( elem->city_id == city_id )
            {
                city = elem;
                break;
            }
        }
        return city;
    }

    long city_manager::get_city_id( const std::string& name )
    {
        long result = -1;
        for( auto elem : city_container )
        {
            if( elem->name == name )
            {
                result = elem->city_id;
                break;
            }
        }
        return result;
    }

    //Return the city_agent for a specific city
    city_agent* city_manager::get_city_agent( long city_id )
    {
        for( auto elem : agents )
        {
            if( elem->get_city_id() == city_id )
            {
                return elem;
            }
        }
        return nullptr;
    }


    city_agent::city_agent( city_manager* manager , city_info_t* city_info ) : mng( manager ), city( city_info )
    {
        LOG("city_agent::city_agent(): New city agent for ", city_info->name, ", city ID: ", city_info->city_id );
    }

    long city_agent::get_city_id()
    {
        return city->city_id;
    }

    std::vector< sf::VertexArray* >* city_agent::get_vertex()
    {
        return city->citymap->get_city_vertex();
    }

    citymap_field_t* city_agent::get_field_at_pos( long x , long y )
    {
        return city->citymap->get_field_at_pos( x , y );
    }

    city_info_t* city_agent::get_city_info()
    {
        return city;
    }

    citymap_t* city_agent::get_city_map()
    {
        return city->citymap;
    }
}
