#ifndef MAP_COMMON_HPP
#define MAP_COMMON_HPP

#include <string>
#include <SFML/Graphics.hpp>

namespace game_map
{
    //This is the structure which needs to be used to configure the graphic apparence for the map
	//On the base of those values the proper vertex are created.
	struct map_viewport_settings_t
	{
	    //Those values are in pixels
	    long map_width,
            map_height;
        //Set to non valid values
        map_viewport_settings_t() : map_width( -1 ) , map_height( -1 )
        {   }
	};
}

namespace map_common
{
	typedef enum field_state
	{
		not_owned = 0,
		owned = 1,
		not_explored = 2,  //It mean that the player has not discovered this field
	}field_state;

	struct field_coordinate
	{
		field_coordinate(unsigned xp = 0, unsigned yp = 0) : x(xp), y(yp) {}
		unsigned x,
			 y;
	};

	struct object_descriptor
	{
		unsigned obj_id;

		std::string name,
		       description;

		long price;

		char symbol;

		//Graphic related stuff
		sf::Color color;
	};
}

namespace citymap
{
    class citymap_t;
    class city_manager;
    //This structure handles the information related with a city (like the owner or the city name) as the citymap_t itself
	struct city_info_t
	{
	    //ID for the city, should be unique
	    long city_id;
	    //Generic information
	    std::string name;
	    map_common::field_coordinate coord; //Position of the city on the map
	    //citymap structure which implement the city
	    citymap_t* citymap;

	    city_info_t( long id , const std::string& city_name );
	    city_info_t() = delete;
	};
    //A city agent is an object responsible for managing operation on a specific city
    class city_agent
    {
        city_info_t* city;
        city_manager* mng;
    public:
        city_agent( city_manager* manager , city_info_t* city_info );
        bool set_city_coord( map_common::field_coordinate coord );
        long get_city_id();
    };
}

#endif
