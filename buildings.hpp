#ifndef BUILDINGS_HPP
#define BUILDINGS_HPP

#include "mytypes.hpp"
#include <string>
#include <fstream>

#define BUILDING_FILENAME "buildings.dat"

namespace buildings
{
	typedef enum building_type_t
	{
		appartment,
		office
	} building_type_t; 

	//Include some common information about the building
	struct building_descriptor_t
	{
		//Generic info
		building_type_t type;

		std::string name,
			description;

		//Basic information about the construction time
		mlong construction_time; //in game cycles

		mlong price;
	};

	struct appartment_descriptor_t
	{
		building_descriptor_t descriptor;
	};

	//Allowed instruction in the building descriptor file
	extern const std::string allowed_instruction[];

	//Handle the available buildings, reading them from the file ecc.
	class building_manager
	{
		int read_building_file();
		short continue_parsing( const std::string& line );
		short get_the_instruction( size_t pos, const std::string& line );
	public:
		building_manager();
	};
}

#endif
