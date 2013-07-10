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

#endif
