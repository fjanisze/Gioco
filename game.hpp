#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#ifndef GAME_HPP
#define GAME_HPP

#include "logging/logger.hpp"
#include "cities.hpp"
#include "population.hpp"
#include "common_structures.hpp"
#include <typeinfo>
#include <list>
#include <iostream>
#include <limits>
#include <process.h>
#include <vector>
#include <exception>
#include <stdlib.h>
#include <string>
#include <ctime>
#include <cmath>
#include <windows.h>
#include <cassert>
#include <memory>
#include <map>
#include <locale>
#include <atomic>

#include "economic/economic.hpp"


using namespace std;

typedef long long mlong;

namespace city_manager
{
	class city;
	struct building_info;
}

namespace game_manager
{
	struct player_game_objects;
}

namespace game_objects
{

	typedef enum object_type
	{
		obj_land = 0,
		obj_building
	}object_type;

	struct object_descriptor
	{
		unsigned obj_id;

		string name,
		       description;

		finance::currency_type price;

		char symbol;
	};

	//Some standard objects
	static const object_descriptor base_land = { 1, "Land" , "Piece of land, nothing more", 100, 'X' };
	static const object_descriptor terrain_grass = { 2, "Grass", "Fresh grass", 200, '.'};
	static const object_descriptor terrain_forest = { 3, "Forest", "A lot of tree", 600 , 'F' };
	static const object_descriptor terrain_dummy = { 4, "DUMMY", "A dummy terrain", 0 , '?' };
	static const object_descriptor terrain_city = { 5, "City", "In this field a city was deployed", 0 , 'C' }; //The value of those fields depend on the items present in the city
	//Some specific objects

	bool is_a_terrain_object(const object_descriptor& obj);
}

namespace player_info
{
	//Type of player
	typedef enum type_of_player
	{
		human_player = 1,
		computer_player
	} type_of_player;

	struct player_info
	{
		string name;
		type_of_player player_type;
	};

	struct coord
	{
		coord(unsigned xp = 0, unsigned yp = 0) : x(xp), y(yp) {}

		unsigned x,
			 y;
	};

	class player_manager
	{
		player_info* player;
	public:
		player_manager( const string& name , type_of_player player_type );
		player_info* get_player_info( );
		std::string get_player_name();
	};
}

namespace game_map
{

	using namespace game_objects;

	static const mlong base_field_value = 1000;
	static const char not_discovered_field_symbol = '?';

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

	//declaration for field_manager
	class field_manager;

	typedef list< object_descriptor* > obj_list_t;

	//map_field collect many information about a field
	struct map_field
	{
		long field_id;
		field_coordinate coord;
		short state; 
		int owner;
		finance::currency_type value;
		field_manager* manager;

		~map_field();
		map_field();
	};

	//Class with calculation support functions
	class map_calculation
	{
		typedef field_coordinate fc;
	public:
		double distance(const fc& point1, const fc& point2);
		long first_closest_field( const fc& origin, const vector< fc >& other_fields );
		fc path_find_next_field( const fc& origin, const fc& dest );
		bool are_coordinates_equal( const fc& point1, const fc& point2 );
		list< field_coordinate > get_field_perimeter( const field_coordinate& origin, int range );
	};

	typedef vector< field_coordinate > vec_field_coord;

	class gameplay_map : public map_calculation
	{
		vector< map_field* > map;
		long map_size;
		long num_of_fields;
		void set_invalid_coord( field_coordinate& coord );
		bool check_field_type_presence( const field_coordinate& coord,  const object_descriptor& expected_obj );
	public:
		field_coordinate find_closest_field_of_type( const field_coordinate& origin, const object_descriptor* type ); 
	public:
		void create_new_map( long size );
		gameplay_map();
		gameplay_map(long size);
		~gameplay_map();
		field_manager* add_obj_to_field(const field_coordinate& coord, const object_descriptor* obj);
		void generate_random_map();
		auto get_fieldmap() -> decltype( map ) { return map; }
		long get_map_size() { return map_size; };
		long calculate_index( const field_coordinate& coord);
		bool are_coord_valid( const field_coordinate& coord);
		field_coordinate find_random_field( const object_descriptor& type );
	public:
		void make_all_map_explored();
		field_manager* create_a_city_at_random_coord( const string& name );
	};

	//field_manager is responsible to managing the objects that lead on a field

	class field_manager
	{
		obj_list_t obj_list; //List of objects present on this field
		char symbol; // graphical symbol for this field
		object_descriptor* create_new_obj_descriptor();
	public:
		field_manager();
		bool add_object(const object_descriptor* obj);
		~field_manager();
	public:
		obj_list_t& get_obj_list();
		char get_field_symbol();
	};
}

////////////////////////////////////////////////////////////////////
//
//
//	The game manager class is responsible for keeping together
//	all the game component. It also stay between the UI and
//	the game functionalities.. 
//
//
////////////////////////////////////////////////////////////////////

namespace game_manager
{
	class game_manager
	{
		static game_manager* instance;
		game_map::gameplay_map   map; //Here we have the game map
		std::map< std::string , player_game_objects* > player_objects;
	public:
		static game_manager* get_instance();
		game_manager();
		~game_manager();
	public:
		void create_test_scenario();
		game_map::gameplay_map* get_the_game_map();
		player_game_objects* get_player_objects( const std::string& player );
		player_game_objects* create_new_player( const std::string& player, player_info::type_of_player player_type );
		std::string get_human_player_name();
	};
};

namespace console_ui
{
	
#define WHITE_INTENSE_COLOR  FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY 
#define FOREGROUND_WHITE FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE

	struct colors 
	{
		long color;
		colors( long possible_color ) : color( possible_color )
		{	}
	};

	struct choice_range
	{
		long down_limit,
		     up_limit;
		long choice;
		choice_range( mlong from, mlong to ) : down_limit( from ) , up_limit( to )
		{	}
	
	};

	static colors color_fore_white( FOREGROUND_WHITE );
	static colors color_fore_red( FOREGROUND_RED );
	static colors color_fore_iwhite( WHITE_INTENSE_COLOR );
	static colors color_fore_green( FOREGROUND_GREEN );
	static colors color_fore_yellow( FOREGROUND_GREEN | FOREGROUND_RED );

	class user_interface;

	typedef vector< string > vec_of_str;
	typedef list< string > list_of_str;

	struct command_pack
	{
		string command; //The command requested by the user
		vec_of_str parms;//Other parameter provided with this command
		//Constructors and other
		command_pack() = default;
		command_pack& operator=( command_pack&& ) = default;
		command_pack( command_pack&& ) = default;
	};

	std::string format_cur_type( const finance::currency_type& value );

	//Some c-out/in operator were overloaded to allow string coloring and other stuff
	std::ostream& operator<<( ostream& out_stream , const colors& color );
	std::istream& operator>>( istream& in_stream , command_pack& cmd );
	std::istream& operator>>( istream& in_stream , choice_range& choice );

	using namespace game_map;

	struct command
	{
		string name;
		int (user_interface::*function)( const command_pack& );
	};

	class console_input
	{
	public:
		command_pack command_input();
	};

	class city_management_ui
	{
		city_manager::city* managed_city; // This is the city which the user is actually managing ( is in the city administration panel )
	public: //Other
		void set_managed_city( city_manager::city* current_city );
	public: // UI operations
		int cm_help( const command_pack& );
		int cm_quit( const command_pack& );
		int cm_list_city_buildings( const command_pack& );
		int cm_build_construction( const command_pack& );
	};

	class user_interface : public console_input , public city_management_ui
	{
		void city_manager_loop( city_manager::city* ct );
		game_manager::player_game_objects* game_obj;
		game_manager::game_manager* game_mng_inst;
		std::string human_player;
	public:
		void print_obj_list(const field_coordinate& coord);
		user_interface();
		int execute_a_command( const command_pack& cmd, command cmd_table[] );
		void input_loop();
	public: // Generic operations
		int draw_map( const command_pack& );
		int quit( const command_pack& );
		int help( const command_pack& );
		int list_player_cities( const command_pack& );
		int enter_city( const command_pack& );
		int wallet( const command_pack& );
		int bank( const command_pack& );
		int population_info( const command_pack& );
		int tax_manager( const command_pack& );
	};

	//List of possible commands
	static command command_list[] = 
	{
		{ "help" , &user_interface::help },
		{ "quit" , &user_interface::quit  },
		{ "drawmap" , &user_interface::draw_map },
		{ "wallet" , &user_interface::wallet },
		{ "citylist", &user_interface::list_player_cities },
		{ "entercity", &user_interface::enter_city  },
		{ "population", &user_interface::population_info },
		{ "taxes", &user_interface::tax_manager },
		{ "bank", &user_interface::bank },
		{ "" , nullptr }
	};

	static command city_manager_command_list[] = 
	{
		{ "help" , &user_interface::cm_help  },
		{ "quit" , &user_interface::cm_quit },
		{ "listbuildings" , &user_interface::cm_list_city_buildings },
		{ "build" , &user_interface::cm_build_construction },
		{ "" , nullptr }
	};
}

#endif
