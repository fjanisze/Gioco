#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2


#include "logging/logger.hpp"
#include "buildings.hpp"

namespace buildings
{
	//Allowed instruction in the building descriptor file
	const std::string allowed_instruction[] =
	{
		"new_descriptor",
		"type",
		"name",
		"description",
		"construction_time",
		"units",
		"unit_capacity",
		"unit_base_price",
		"building_price",
		"end",
		""
	};
}

////////////////////////////////////////////////////////////////////
//
//
//	Follow the implementation for the building manager
//
//
////////////////////////////////////////////////////////////////////

namespace buildings
{
	using namespace std;

	building_manager::building_manager()
	{
		LOG("building_manager::building_manager(): Starting..");
		read_building_file();
	}

	//The function read from the building file the list of buildings and create the appropriate
	//data structure.
	int building_manager::read_building_file()
	{
		LOG("building_manager::read_building_file(): Building file ",BUILDING_FILENAME );
	
		int amount_of_buildings = 0 , line_counter = 1;
		std::ifstream in_file( BUILDING_FILENAME );

		if( in_file )
		{
			std::string buffer;
			while( !std::getline( in_file , buffer ).eof() )
			{
				//got a new line, start parsing if contain instructions
				short pos;
				if( ( pos = continue_parsing( buffer ) ) >= 0 )
				{
					short instruction = get_the_instruction( pos , buffer );
					if( instruction < 0 )
					{
						//Skip the whole building definition,
						//This mean that skip everything until the instruction 'end'
						LOG_ERR("building_manager::read_building_file(): Error at line ",line_counter," skipping the whole building definition");
					}
					else
					{
						cout<<buffer<<" --> "<<instruction<<endl;
					}
				}
				++line_counter;
			}
		}
		else
		{
			LOG_ERR("building_manager::read_building_file(): Unable to open the building file");
			return -1; //not goodd
		}

		return amount_of_buildings;
	}
	
	//If the line contains valid instructions the function return the position where
	//the instruction begin, otherwise npos
	short building_manager::continue_parsing( const string& line )
	{
		ELOG("building_manager::continue_parsing(): Input line \"",line,"\"");
		short pos = -1, line_size = line.size();
		for( short i = 0 ; i < line_size ; i++)
		{
			if( line[ i ] == '@' )
			{
				pos = i + 1;
				break;
			}
		}
		return pos;
	}

	//Return the insturction index (as per the table)
	short building_manager::get_the_instruction( size_t pos, const string& line )
	{
		short index = 0;
		size_t end_instr_pos = line.find_first_of( ":;" , pos + 1 );
		if( end_instr_pos == string::npos )
		{
			LOG_ERR("building_manager::get_the_instruction(): Unknow format identified in: ",line );
			return -1;
		}
		std::string instruction = line.substr( pos , end_instr_pos - pos );
		for( auto& current_instr : allowed_instruction )
		{
			if( current_instr == instruction )
			{
				return index;
			}
			++index;
		}
		LOG_ERR("building_manager::get_the_instruction(): Unknow instruction (",instruction,") in: ",line );
		return -1;
	}
}
