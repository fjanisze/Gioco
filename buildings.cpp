#define LOGGING_LEVEL_1
//#define LOGGING_LEVEL_2


#include "logging/logger.hpp"
#include "buildings.hpp"

namespace buildings
{
	const string building_types_str[] =
	{
		"APPARTMENT",
		"OFFICE",
		""
	};

	//Indexes for the instructions
	static const int new_instruction_index = 0;
	static const int type_instruction_index = 1;
	static const int name_instruction_index = 2;
	static const int desc_instruction_index = 3;
	static const int consttime_instruction_index = 4;
	static const int units_instruction_index = 5;
	static const int unitcapa_instruction_index = 6;
	static const int unitprice_instruction_index = 7;
	static const int price_instruction_index = 8;
	static const int end_instruction_index = 9;

	//Allowed instruction in the building descriptor file
	const std::string allowed_instruction[] =
	{
		"new_descriptor", //0
		"type",
		"name",
		"description",
		"construction_time",
		"units",
		"unit_capacity",
		"unit_base_price",
		"building_price",
		"end", //9
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

	long building_manager::building_id = 0;

	building_manager::building_manager()
	{
		LOG("building_manager::building_manager(): Starting..");
		current_instruction = nullptr;
		read_building_file();
	}

	building_manager::~building_manager()
	{
		for( auto elem : available_appartments )
		{
			delete elem;
		}
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
			bool skip_to_next_descriptor = false;
			while( !std::getline( in_file , buffer ).eof() )
			{
				//got a new line, start parsing if contain instructions
				short pos;
				if( ( pos = continue_parsing( buffer ) ) >= 0 )
				{
					std::pair< short, string > instruction = std::move( get_the_instruction( pos , buffer ) );
					if( skip_to_next_descriptor )
					{
						//If we are here, it means that for some reason this
						//building descriptor should be skipped
						if( instruction.first != new_instruction_index )
						{
							LOG_WARN("building_manager::read_building_file(): Skipping: ",buffer );
							continue; //Waiting the next 'begin' instruction
						}
						skip_to_next_descriptor = false;
					}

					if( instruction.first < 0 )
					{
						//Skip the whole building definition,
						//This mean that skip everything until the instruction 'end'
						LOG_ERR("building_manager::read_building_file(): Error at line ",line_counter," skipping the whole building definition");
						skip_to_next_descriptor = true;
					}
					else
					{
						remove_comments( instruction.second );
						//Execute the instruction
						skip_to_next_descriptor = !execute_instruction( instruction );
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

	bool building_manager::check_if_current_instr_is_valid()
	{
		return current_instruction != nullptr;
	}

	//Parse the fields and execute the operation, return true is the executing can proceed
	bool building_manager::execute_instruction( std::pair< short, string >& instr )
	{
		ELOG("building_manager::execute_instruction(): Executing instruction, index: ",instr.first,", parm: ",instr.second );
		bool next_field = true;
		//Check if the 'new' instruction is the first coming
		if( !check_if_current_instr_is_valid() && instr.first != new_instruction_index )
		{
			//This is not good, a 'new_descriptor' is missing
			LOG_ERR("building_manager::execute_instruction(): A 'new_descriptor' instruction is expected first!");
			next_field = false;
		}
		else if( check_if_current_instr_is_valid() && instr.first == new_instruction_index )
		{
			//Again a new instruction, skipping.
			LOG_WARN("building_manager::execute_instruction(): A 'new_descriptor' was again identified.. Skipping");
		}
		else
		{
			remove_spaces( instr.second );
			//Go ahead
			switch( instr.first )
			{
			case new_instruction_index:
				//Ok, on the base of the field type
				current_instruction = new(nothrow) all_information_t; //Do not handle the exception here.
				assert( current_instruction != nullptr );
				break;
			case type_instruction_index:
				current_instruction->general.type = get_proper_type( instr.second );
				break;
			case name_instruction_index:
				current_instruction->general.name = instr.second;
				break;
			case desc_instruction_index:
				current_instruction->general.description = instr.second;
				break;
			case consttime_instruction_index:
				current_instruction->general.construction_time = atoi( instr.second.c_str() );
				break;
			case units_instruction_index:
				current_instruction->units = atoi( instr.second.c_str() );
				break;
			case unitcapa_instruction_index:
				current_instruction->unit_capacity = atoi( instr.second.c_str() );
				break;
			case unitprice_instruction_index:
				current_instruction->unit_price = atoi( instr.second.c_str() );
				break;
			case price_instruction_index:
				current_instruction->general.price = atoi( instr.second.c_str() );
				break;
			case end_instruction_index:
				finalize_instruction();
				delete current_instruction;
				current_instruction = nullptr;
				break;
			default:
				LOG_ERR("building_manager::execute_instruction(): Unknow index ", instr.first );
				delete current_instruction;
				next_field = false;
				break;
			}
		}
		return next_field;
	}

	//This function should be called when the 'end' tag if found in the building file
	void building_manager::finalize_instruction()
	{
		ELOG("building_manager::finalize_instruction(): Completing ");
		switch( current_instruction->general.type )
		{
		case building_type_t::appartment:
			add_new_appartment();
			break;
		case building_type_t::office:
			break;
		};
	}

	void building_manager::add_new_appartment()
	{
		appartment_descriptor_t* appartment = new appartment_descriptor_t;
		assert( appartment != nullptr );

		appartment->descriptor = current_instruction->general;
		appartment->descriptor.obj_id = ++building_id;
		appartment->units = current_instruction->units;
		appartment->unit_capacity = current_instruction->unit_capacity;
		appartment->unit_price = current_instruction->unit_price;

        LOG("building_manager::add_new_appartment(): Adding appartment: \"",current_instruction->general.name,"\", ID:",appartment->descriptor.obj_id);
		available_appartments.push_back( appartment );
	}

	//From the beginning and the end of the string
	void building_manager::remove_spaces( string& line )
	{
		while( line[0] == ' ' )
		{
			line.erase( 0 , 1 );
		}
		while( line.back() == ' ' )
		{
			line.pop_back();
		}
	}

	building_type_t building_manager::get_proper_type( const string& cmd )
	{
		for( short i = 0 ; !building_types_str[ i ].empty() ; i++ )
		{
			if( cmd.find( building_types_str[ i ] ) != string::npos )
				return building_type_t( i );
		}
		return building_type_t::error;
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
	//The function returns a pair, first contain the index number of the instruction, second the rest of the argument for the instruction
	std::pair< short, string > building_manager::get_the_instruction( size_t pos, const string& line )
	{
		short index = 0;
		std::pair< short, string > return_value( -1 , "" );
		size_t end_instr_pos = line.find_first_of( ":;" , pos + 1 );
		if( end_instr_pos == string::npos )
		{
			LOG_ERR("building_manager::get_the_instruction(): Unknow format identified in: ",line );
			return std::move( return_value );
		}
		std::string instruction = line.substr( pos , end_instr_pos - pos );
		for( auto& current_instr : allowed_instruction )
		{
			if( current_instr == instruction )
			{
				return_value.first = index;
				return_value.second = line.substr( end_instr_pos + 1 );
				return std::move( return_value );
			}
			++index;
		}
		LOG_ERR("building_manager::get_the_instruction(): Unknow instruction (",instruction,") in: ",line );
		return std::move( return_value );
	}

	//Remove the comments from the line
	void building_manager::remove_comments( string& line )
	{
		size_t pos = line.find_first_of("#");
		if( pos != string::npos )
		{
			line = line.substr( 0 , pos );
		}
	}

	//Return a vector with all the appartment available
	std::vector< appartment_descriptor_t* >* building_manager::get_all_the_appartment()
	{
	    return &available_appartments;
	}
}

namespace constructions
{

    construction_handler_t construction_manager::construction_handler_id = 0;
    ////////////////////////////////////////////////////////////////////
    //
    //
    //	Follow the implementation for construction_t and construction_manager
    //
    //
    ////////////////////////////////////////////////////////////////////

    int under_construction_obj_t::trigger_event( long event_id )
    {
        ELOG("under_construction_obj_t::trigger_event(): Event ID: ",event_id,", for the obj ID:",construction->obj_id );
    }

    //Return the next unique handler ID for the construction
    construction_handler_t construction_manager::get_next_hnd_id()
    {
        return ++construction_handler_id;
    }

    //Start the construction of a certain building on a specific field
    construction_handler_t construction_manager::start_construction( long building_id, long city_id, long field_id )
    {
        construction_handler_t handler = get_next_hnd_id();
        LOG("construction_manager::start_construction(): New construction, building ID:",building_id,",City ID:",city_id,",Construction handler ID:",handler );
        return handler;
    }
}





























