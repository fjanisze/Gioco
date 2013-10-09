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
#include "map\map_common.hpp"
#include "construction_common_struct.hpp"

using namespace std;

#define BUILDING_FILENAME "buildings.dat"

namespace constructions
{
    typedef mlong construction_handler_t;


	//Needed to collect all the information during the instruction parsing
	struct all_information_t
	{
		construction_info general;
		long units,
		     unit_capacity;

		mlong unit_price;

		all_information_t() = default;
	};

	//Allowed instruction in the building descriptor file
	extern const std::string allowed_instruction[];

	//Handle the available buildings, reading them from the file ecc.
	class construction_manager
	{
        static construction_handler_t construction_handler_id;
		static long building_id;
		std::vector< construction_t* > available_constructions;
	private:
		all_information_t* current_instruction;
		construction_type_t get_proper_type( const string& cmd );
	private:
	    long get_new_building_id();
		int read_building_file();
		short continue_parsing( const std::string& line );
		std::pair< short, string > get_the_instruction( size_t pos, const std::string& line );
		void remove_comments( string& line );
		void remove_spaces( string& line );
		bool check_if_current_instr_is_valid();
		void finalize_instruction();
		void add_new_apartment();
		bool execute_instruction( std::pair< short, string >& instr );
        construction_handler_t get_next_hnd_id();
	    construction_t* create_new_construction( construction_t* construction_source );
	public:
		construction_manager();
		~construction_manager();
    public:
        std::vector< construction_t* >* get_all_construction();
        construction_t* get_construction_obj( long obj_id );
        construction_handler_t start_construction( long building_id, long city_id , citymap::citymap_field_t* field );
	};
}



#endif





























