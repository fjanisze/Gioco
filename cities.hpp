#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#ifndef CITIES_HPP
#define CITIES_HPP

#include "logging/logger.hpp"
#include <string>
#include <vector>
#include <map>
#include <cassert>
#include "events.hpp"
#include "economic.hpp"
#include "population.hpp"

typedef long long mlong;

namespace game_manager
{
	class player_game_objects;
	class game_manager;
}


namespace city_manager
{

	struct building_info;

	//Some buildings when constructed or destroyed impact the game, this class
	//Contaings the function invoked when some action are triggered
	class building_event_action
	{
	public:
		virtual void construction_completed( building_info* building, game_manager::player_game_objects* player_objects ) = 0; //Is invoked when the construction is completed
		building_event_action();
	};

	//Implementation for the welfare administration building
	class civil_welfare_office_actions : public building_event_action
	{
	public:
		void construction_completed( building_info* building, game_manager::player_game_objects* player_objects );
	};

	//Implementation for the small commercial building
	class civil_small_poor_commercial_actions : public building_event_action
	{
	public:
		void construction_completed( building_info* building, game_manager::player_game_objects* player_objects );
	};

	//Implementation for the small poor market
	class civil_small_poor_market_actions : public building_event_action
	{
	public:
		void construction_completed( building_info* building, game_manager::player_game_objects* player_objects );
	}; 

	typedef enum building_type
	{
		habitable,
		workplace
	} building_type; 

	//City building descriptors.
	//Those descriptors are used to provide basic information about the buildings
	struct building_descriptor
	{
		int building_id;
		string name;
		string description;

		building_type type;

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
		string city;
		string employer_name;
		job_market::job_entity* job; //Who is working here?
		long employed_people;
		workplace_descriptor() : job( nullptr ), employed_people( 0 )
		{	}
	};

	//Some basic building definitions
	extern const building_descriptor civil_small_house;
	extern const building_descriptor civil_medium_house;
	extern const building_descriptor civil_small_appartment;
	extern const building_descriptor civil_medium_appartment;


	//Special buildings and offices
	extern const building_descriptor civil_small_poor_commercial_building;
	extern const building_descriptor civil_small_poor_market;
	extern const building_descriptor civil_welfare_administration_office;

	extern const building_descriptor* buildings_table [];

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

	//Responsible for managing the workplaces
	class workplace_manager
	{
	public:
		workplace_descriptor* create_new_workplace( building_info* building );
	};

	class city;

	//This class is responsible for the city construction management
	class construction_management : public events::event_entity , public workplace_manager
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

	extern const long default_city_size;

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

#endif 
