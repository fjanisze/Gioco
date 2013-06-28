#ifndef MAP_COMMON_HPP
#define MAP_COMMON_HPP

#include <string>

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
	};
}

#endif
