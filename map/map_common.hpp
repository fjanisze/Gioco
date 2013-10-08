#ifndef MAP_COMMON_HPP
#define MAP_COMMON_HPP

#include <string>
#include <SFML/Graphics.hpp>
#include "../mytypes.hpp"

namespace game_map
{
    //This is the structure which needs to be used to configure the graphic apparence for the map
	//On the base of those values the proper vertex are created.
	struct map_viewport_settings_t
	{
	    //Offset from the left top corner of the window
	    long map_x_offset,
            map_y_offset;
	    //Those values are in pixels
	    long map_width,
            map_height;
        //Set to non valid values
        map_viewport_settings_t() : map_width( -1 ) , map_height( -1 ),
                    map_x_offset( -1 ) , map_y_offset( -1 )
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

namespace cities
{
    struct city_info_t;
}

namespace citymap
{
    class citymap_t;
    //Possible type of field
	typedef enum field_type_t
	{
	    grass_field, //An empty terrain with grass, is possible to built such a fields
	    custom_field, //It mean that something was build here..
	} field_type_t;

	struct city_field_descriptor
	{
	    //Should be unique
		mlong obj_id;
		//Information about the field, name ecc.
		std::string name,
                desc;
        //Describe the kind of terrain we are facing with
        field_type_t field_type;
	};

    //Contains some information on the city
	struct citymap_info_t
	{
	    int size;
	    long amount_of_fields; //Which is just size * size

	    //Utility constructors ecc
	    citymap_info_t();
	    void set_info( int map_size );
	};

	//City field
	struct citymap_field_t
	{
	    //ID for the field
	    mlong field_id;

	    //Descriptor, with generic information on the field
	    city_field_descriptor* descriptor;

	    //Coordinate of the field
	    map_common::field_coordinate coord;

	    //here are contained the information about the building (if any) available on this field.
	//    constructions::construction_t* construction;

	    citymap_field_t();
	    city_field_descriptor* new_descriptor() throw( std::bad_alloc );
	    ~citymap_field_t();
	};
}

namespace cities
{
    using namespace citymap;
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
        std::vector< sf::VertexArray* >* get_vertex();
        citymap_field_t* get_field_at_pos( long x , long y );
        city_info_t* get_city_info();
    };
}

#endif
