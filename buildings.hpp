#ifndef BUILDINGS_HPP
#define BUILDINGS_HPP

#include "mytypes.hpp"
#include "events.hpp"
#include <string>
#include <fstream>
#include <utility>
#include <cassert>
#include <vector>
#include <map>

using namespace std;

#define BUILDING_FILENAME "buildings.dat"

namespace buildings
{
	typedef enum building_type_t
	{
		appartment,
		office,
		error
	} building_type_t;

	extern const string building_types_str[];

	//Include some common information about the building
	struct building_descriptor_t
	{
		long obj_id;

		//Generic info
		building_type_t type;

		std::string name,
			description;

		//Basic information about the construction time
		mlong construction_time; //in game cycles

		mlong price;

		building_descriptor_t() = default;
	};

	struct appartment_descriptor_t
	{
		building_descriptor_t descriptor;

		long units,
		     unit_capacity;

		mlong unit_price;

		appartment_descriptor_t() = default;
	};

	//Needed to collect all the information during the instruction parsing
	struct all_information_t
	{
		building_descriptor_t general;
		long units,
		     unit_capacity;

		mlong unit_price;

		all_information_t() = default;
	};

	//Allowed instruction in the building descriptor file
	extern const std::string allowed_instruction[];

	//Handle the available buildings, reading them from the file ecc.
	class building_manager
	{
		static long building_id;
		std::vector< appartment_descriptor_t* > available_appartments;
	private:
		all_information_t* current_instruction;
		building_type_t get_proper_type( const string& cmd );
	private:
		int read_building_file();
		short continue_parsing( const std::string& line );
		std::pair< short, string > get_the_instruction( size_t pos, const std::string& line );
		void remove_comments( string& line );
		void remove_spaces( string& line );
		bool check_if_current_instr_is_valid();
		void finalize_instruction();
		void add_new_appartment();
		bool execute_instruction( std::pair< short, string >& instr );
	public:
		building_manager();
		~building_manager();
    public:
        std::vector< appartment_descriptor_t* >* get_all_the_appartment();
	};
}

namespace constructions
{
    //This is a construction object, which should be present on a field
    struct construction_t
    {
        long obj_id;
        //Type of construction
        buildings::building_type_t type;
        //Generic information, from the relative descriptor
        std::string name,
                    description;
        //Unit information
        long unit_size,
            unit_capacity; //The total capacity of the building is unit_size * unit_capacity

        //The price in the descriptor may be not valid anymore, perks can influence the price
        mlong unit_price;
    };

    //This is a support entity used
    class under_construction_obj_t : public events::event_entity
    {
        construction_t* construction;
    public:
        under_construction_obj_t();
        int trigger_event( long event_id );
    };

    typedef mlong construction_handler_t;

    //This class manage the construction procedure for the cities
	class construction_manager
	{
	    static construction_handler_t construction_handler_id;
	    std::map< construction_handler_t , under_construction_obj_t* > obj_under_construction;
	    construction_handler_t get_next_hnd_id();
	    under_construction_obj_t* new_under_construction_obj( long building_id );
    public:
        construction_handler_t start_construction( long building_id, long city_id, long field_id );
        ~construction_manager();
	};
}

#endif





























