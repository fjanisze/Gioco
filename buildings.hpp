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

using namespace std;

#define BUILDING_FILENAME "buildings.dat"

namespace constructions
{
	typedef enum construction_type_t
	{
		appartment,
		office,
		error
	} construction_type_t;

	extern const string building_types_str[];

	//Include some common information about the construction, used during the construction parsing.
	struct construction_info
	{
		long obj_id;

		//Generic info
		construction_type_t type;

		std::string name,
			description;

		//Basic information about the construction time
		mlong construction_time; //in game cycles

		mlong price;

		construction_info() = default;
	};

    //This is a construction object, which should be present on a field
    class construction_t
    {
    protected:
        long obj_id;
        //Type of construction
        construction_type_t type;
        //Generic information, from the relative descriptor
        std::string name,
                    description;
        //Unit information
        long unit_size,
            unit_capacity; //The total capacity of the building is unit_size * unit_capacity

        //The price in the descriptor may be not valid anymore, a perks can influence the price
        mlong unit_price;
    public:
        construction_type_t get_construction_type();
        long get_obj_id();
        std::string get_name();
        friend class construction_manager;
    };

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
		int read_building_file();
		short continue_parsing( const std::string& line );
		std::pair< short, string > get_the_instruction( size_t pos, const std::string& line );
		void remove_comments( string& line );
		void remove_spaces( string& line );
		bool check_if_current_instr_is_valid();
		void finalize_instruction();
//		void add_new_appartment();
		bool execute_instruction( std::pair< short, string >& instr );
        construction_handler_t get_next_hnd_id();
	//    under_construction_obj_t* new_under_construction_obj( long building_id );
	    construction_t* create_new_construction( constructions::construction_t* building_obj );
	public:
		construction_manager();
		~construction_manager();
    public:
        std::vector< construction_t* >* get_all_construction();
        construction_t* get_building_obj( long obj_id );
        construction_handler_t start_construction( long building_id, long city_id , citymap::citymap_field_t* field );
	};
}



#endif





























