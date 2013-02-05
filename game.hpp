#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2
#define GAME_SPEED 20

#ifndef GAME_HPP
#define GAME_HPP

#include "logging/logger.hpp"
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

#include "economic.hpp"

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

namespace population
{

#define BASE_GROW_RATION 1.1545084972495 //Take the decimal part of the golden ratio and divide them by 4
#define STARVING_RATIO_THRESHOLD 0.25 //If more than 25% of the population is starving, then a quick depopulation trend will begin

	typedef enum population_class
	{
		class_0 = 0,//Very low qualified and poor persons.
		class_1,
		class_2,
		class_3,
		class_4,
		class_5 //High qualified and rich persons
	} population_class;

	class population_manager;

	class population_entity
	{
		static mlong entity_next_id; 
		mlong entity_id;
		long population,
		     max_population;
		population_class pop_class; 
		economics::economic_unit* entity_eu; //Economic_unit for this population entity, 
	public:
		long get_population()
		{	return population;	}
		long get_max_population()
		{	return max_population;	}
		long change_population( long amount_of_pop );
		long set_to_max_population();
		bool can_grow_more()
		{	return population < max_population;	}
		bool have_zero_population()
		{	return population == 0;		}
		long left_to_max_population()
		{	return max_population - population;	}
		population_entity();
		economics::currency_type get_unit_net_revenue();
		economics::economic_unit* get_economic_entity()
		{	return entity_eu;	}
		friend class population_manager;
	};

	typedef std::list< population_entity* > pop_entity_list;

	//A collection refers to a certain population which belong to a specific player
	//each player has its own collection, and each collection may have its specific caracteristic and
	//distributions, like between gender or age.
	struct collection
	{
		std::mutex access_mutex;
		pop_entity_list population;
		mlong total_population, //Total population sum
		      max_population;
		mlong calculate_population();
		~collection();
	};

	class population_manager
	{
		long double population_grow_cycle_factor,
			    population_grow_cycle_inc,
			    population_grow_cycle_decrease_attenuation;
		mlong	    population_grow_cycle_trigger_threshold;
		void increase_grow_cycle_factor();
		double get_grow_cycle_factor();
		game_manager::player_game_objects* player_obj;
	private://Misc structures and containers
		std::string player_name;
		collection* collections;
		std::map< std::string , std::vector< population_entity* > > city_population_set;
		mlong total_population,
		      max_population;
		mlong calculate_population();
		mlong calculate_population_delta( );
		mlong apply_economic_var_to_delta( mlong delta );
		long increase_population( long delta, collection* coll );
		long decrease_population( long delta, collection* coll );
	public:
		bool create_new_collection();
		population_entity* create_new_population_entity( long max_population, population_class p_class , const economics::expense_and_cost& costs, const economics::revenues_and_profits& revenues  );
		bool add_population_entity( population_entity* pop, const std::string& city );
		mlong review_population() ;
		mlong get_total_population();
		mlong get_max_population();
		std::pair< mlong , mlong > get_city_population( const std::string& city );
	public:
		population_manager( const std::string& player );
		~population_manager();
		void set_player_game_obj( game_manager::player_game_objects* game_obj );
	};
}

namespace events
{
	class event_entity
	{
	public:
		virtual int trigger_event( long event_id ) = 0;
	};

	typedef enum event_execution
	{
		execute_oneshot,
		execute_repeat,
	} event_execution;

	class event_manager;

	typedef struct timer_type
	{
	private:
		long timer_value;
	public:
		timer_type& operator=( const long value );
		bool operator>( const long value );
		bool operator<( const long value );
		long get_time_left() const;
		friend class event_manager;
	} timer_type;

	typedef struct event_obj
	{
		event_entity* event_pointer;
		long event_id;
		timer_type* timer;
		timer_type timer_copy;
		event_execution execution_rule;
		event_obj() : event_id( 0 ) , timer( nullptr ) , event_pointer( nullptr )
		{}
	} event_obj;

	class event_manager
	{
		static event_manager* instance;
		std::mutex event_mutex;
		std::list< event_obj > events;
		long next_event_id;
		event_obj* get_event_obj( long event_id );
		bool decrease_and_check( event_obj* );
	public:
		static event_manager* get_instance();
		event_manager();
		long register_event( event_entity* target_event ); 
		bool unregister_event( long event_id );
		bool start_event_countdown( long event_id, timer_type* timer ,event_execution exec_type ); 
	public:
		void main_loop(); 
	};
}

namespace player_info
{
	struct player_info;
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

namespace city_manager
{
	using namespace game_objects;

	//The next enum define the type of building buildable
	typedef enum building_type 
	{
		civil_appartment = 1,
		civil_office
	} building_type;

	//Some buildings when constructed or destroyed impact the game, this class
	//Contaings the function invoked when some action are triggered
	class building_event_action
	{
	public:
		virtual void construction_completed() = 0; //Is invoked when the construction is completed
	};

	//Implementation for the welfare administration building
	class civil_welfare_office_actions : public building_event_action
	{
	public:
		void construction_completed();
	};

	//City building descriptors.
	//Those descriptors are used to provide basic information about the buildings
	struct building_descriptor
	{
		int building_id;
		string name;
		string description;

		bool allowed_multiple_builds; //Are multiple of those building allowed in the same city?
		bool one_build_per_player; //Can the player have more than one of those buildings?

		finance::currency_type price,
				rental_price, //How much cost to rent this building? Each person living in the building will pay this price
				maintanance_cost; //Is up to the player which build the building to pay the maintanance_cost

		long construction_time; //In game tick
		long population_capacity; //Max amount of population allowed in this building

		building_event_action* actions;//Possible actions related with this building
		population::population_class pop_class;
		~building_descriptor();
	};

	//This structured is used to describe a workplace source, like and office or a factory
	//the amount of workplaces is taken from the population_capacity field in the building descriptor
	struct workplace_descriptor
	{
		finance::currency_type employee_salary; //Payed yearly, this is a gross value
	};

	//Some basic building definitions
	static const building_descriptor civil_small_house = { 1 , "Small house", "Small mono family house", true, false, 350000, 650, 3000,  20, 7 , nullptr , population::population_class::class_0 };
	static const building_descriptor civil_medium_house = { 2 , "Medium house", "This is a little bigger house, for up to three families", true,false, 1350000, 700, 7500, 35, 20 , nullptr , population::population_class::class_0 };
	static const building_descriptor civil_small_appartment = { 3 , "Appartment", "Building with a set of small appartment", true,false, 14000000, 720, 200000 , 110 , 300 , nullptr , population::population_class::class_0 };
	static const building_descriptor civil_medium_appartment = { 4 , "Medium Appartment", "Building with a set of medium size apparment", true,false, 32000000, 550, 500000, 250 , 800 , nullptr , population::population_class::class_0 };

	//Special buildings and offices
	static const building_descriptor civil_welfare_administration_office = { 5 , "Welfare administration" , "This construction is needed to make the social poor welfare work", false, true, 25000000, 0, 3500000, 220 , 600, new civil_welfare_office_actions, population::population_class::class_1  };

	static const building_descriptor* buildings_table [] =
	{
		&civil_small_house,
		&civil_medium_house,
		&civil_small_appartment,
		&civil_medium_appartment,
		&civil_welfare_administration_office,
		nullptr
	};

	//This struct contain all the relevant information for a building
	struct building_info
	{
		const building_descriptor* descriptor; // General information about the building
		workplace_descriptor* workplace_desc; //If null, this building is not a source of workplaces.

		events::timer_type remaining_construction_time; //Time left until the construction will be ready.
		population::population_entity* population;
		finance::currency_type building_value;

		building_info();
		~building_info();
		const string& get_name() const 
		{ 
			assert( descriptor != nullptr );
			return descriptor->name; 
		}
	};

	class city;

	//This class is responsible for the city construction management
	class construction_management : public events::event_entity
	{
		game_manager::player_game_objects* player_objects;
		vector< building_info* > constructions; //Those are the buildings available on the city area 
		map< long , building_info* > construction_triggers;
		city* this_city;
		void construction_completed( building_info* building );
		bool is_construction_ongoing() const;
		bool is_the_building_present( const building_descriptor* building ) const;
	public:
		bool start_building_construction( const building_descriptor* building );
		int is_possible_to_build( const building_descriptor* building  ) const;
		const vector< building_info* >& get_construction_list() const;
		void set_player_objects( game_manager::player_game_objects* obj );
	public:
		int trigger_event( long event_id );
		construction_management();
		~construction_management();
	};

	//This structure describe on cell of the city which reppresent an 'area',
	//each area can be used to build a construction ( buildings etc ).
	typedef struct city_area //TODO
	{
		//There is a building on this area?
		building_info* construction;
		city_area() : construction( nullptr )
		{	}
	}city_area;

	//A city can be considered as a square of a certain size, the default size
	//of the city 'square' edge is defined by default_city_size
	static const long default_city_size = 5;

	//This class is responsible for handling the map of a city
	class city_map_management //TODO
	{
		vector< city_area* > city_map;
	public:
		void create_empty_map( long size = default_city_size );
		~city_map_management();
	};

	//City reppresenting object
	class city : public construction_management, private city_map_management
	{
		string city_name,
		       city_owner;
	public:
		string get_city_name() { return city_name; }
		string get_city_owner() { return city_owner; }	
		economics::currency_type get_city_net_revenue();
	public:
		city( const string& name, const string& owner );
		~city();
	};

	//This class is responsible for managing the cities
	class city_manager
	{
		vector< city* > cities;
		std::string player_name;
	public:
		city_manager( const std::string& player );
		~city_manager();
	public:
		city* get_city_info( const string& name );
		city* create_new_city( const string& name );
		vector< city* >* get_the_cities();
	};
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

	struct player_game_objects
	{
		player_info::player_manager*    player;
		city_manager::city_manager*     cities;
		population::population_manager* population;
		economics::economy_manager*     economics;
	};

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
