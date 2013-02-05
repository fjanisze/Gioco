#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#ifndef POPULATION_HPP
#define POPULATION_HPP

#include "logging/logger.hpp"
#include "economic.hpp"
#include "common_structures.hpp"
#include <string>
#include <vector>
#include <map>
#include <cassert>
#include <list>
#include <mutex>


#define BASE_GROW_RATION 1.1545084972495 //Take the decimal part of the golden ratio and divide them by 4
#define STARVING_RATIO_THRESHOLD 0.25 //If more than 25% of the population is starving, then a quick depopulation trend will begin

typedef long long mlong;


namespace population
{
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
		long get_population();
		long get_max_population();
		long change_population( long amount_of_pop );
		long set_to_max_population();
		bool can_grow_more();
		bool have_zero_population();
		long left_to_max_population();
		population_entity();
		economics::currency_type get_unit_net_revenue();
		economics::economic_unit* get_economic_entity();
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

#endif 
