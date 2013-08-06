#ifndef CITIES_HPP
#define CITIES_HPP

#include "map/city_map.hpp"
#include "logging/logger.hpp"
#include <SFML/Graphics.hpp>
#include "map/map_common.hpp"
#include "map/city_map.hpp"

namespace cities
{
    using namespace citymap;
    class city_agent;

    //Handle the cities
	class city_manager
	{
	    static long next_city_id;
	    std::vector< city_info_t* > city_container; //All the cities are contained in this vector
	    //Each city have its own city_agent
	    std::vector< city_agent* > agents;
	    //The viewport settings are needed for the city map creation
	    game_map::map_viewport_settings_t viewport_settings;
	    long get_next_city_id();
	    city_info_t* find_city_info( long city_id );
	    city_agent* create_new_agent( city_info_t* city_info );
    public:
        city_agent* get_city_agent( long city_id );
        long get_city_id( const std::string& name );
        city_manager( game_map::map_viewport_settings_t viewport );
        city_agent* create_new_city( const std::string& name , long size );
	};
}

#endif
