#include "population.hpp"
#include "game.hpp"

namespace population
{
	mlong population_entity::entity_next_id = 0;


	long population_entity::get_population()
	{
		return population;
	}
	long population_entity::get_max_population()
	{
		return max_population;
	}
	bool population_entity::can_grow_more()
	{	
		return population < max_population;
	}
	bool population_entity::have_zero_population()
	{	
		return population == 0;	
	}
	long population_entity::left_to_max_population()
	{	
		return max_population - population;
	}

	economics::economic_unit* population_entity::get_economic_entity()
	{
		return entity_eu;
	}

	collection::~collection()
	{
		LOG("collection::~collection(): Destroying collection");
		for( auto elem : population )
		{
			delete elem;
		}
	}

	mlong collection::calculate_population()
	{
		ELOG("collection::calculate_population(): population container size: ",population.size() );
		total_population = max_population = 0;
		for( auto elem : population )
		{
			total_population += elem->get_population();
			max_population += elem->get_max_population();
		}
		return total_population;
	}

	population_entity::population_entity() : population(0), max_population( 0 ),
						entity_id( entity_next_id++ ) , entity_eu( nullptr ) 
	{	}

	//The function return the amount of units which was not possible to add or remove from this population_entity
	//amount_of_pop can be positive or negative, hence, the population can be increased or decreased.
	long population_entity::change_population( long amount_of_pop )
	{
		long delta = 0;
		ELOG("population_entity::change_population(): max_population: ",max_population,", change of: " ,amount_of_pop, ", actual population: ",population );
		if( amount_of_pop > 0 )
		{
			if( amount_of_pop + population > max_population )
			{
				delta = ( amount_of_pop + population ) - max_population; //Un-asigned
				population = max_population;
			}
			else
			{
				population += amount_of_pop;
			}
		}
		else
		{
			if( amount_of_pop + population < 0 )
			{
				delta = std::abs( amount_of_pop + population );
				population = 0;
			}
			else
			{
				population += amount_of_pop;
			}
		}

		return delta;
	}

	//The function return to max population, and return the amount of 'people' added
	long population_entity::set_to_max_population()
	{
		long difference = max_population - population;
		population = max_population;
		return difference;
	}

	economics::currency_type population_entity::get_unit_net_revenue()
	{
		if( entity_eu == nullptr )
		{
			return 0;
		}
		return entity_eu->get_unit_net_revenue();
	}

	population_manager::population_manager( const std::string& player )
	{
		collections = nullptr;
		player_name = player;
		create_new_collection();
		population_grow_cycle_factor = 0;
		population_grow_cycle_inc = 0.05; //This value will determine how long will be the positive or negative cycle in the population increase/decrease
		population_grow_cycle_decrease_attenuation = 0.03; //Smaller is this value, then smaller will be the impact of a negative cycle in the population growth
		population_grow_cycle_trigger_threshold = 20; //If less than population_grow_cycle_trigger_threshold are living, then the population grow cycle will not be calculated
		LOG("population_manager::population_manager(): Has been constructed for ",player,", grow_cycle_factor: ",population_grow_cycle_factor,", population_grow_cycle_inc: ",population_grow_cycle_inc);
	}

	population_manager::~population_manager()
	{
		LOG("population_manager::~population_manager(): Cleaning...");
		for( auto elem : city_population_set )
		{
			for( auto set_elem : elem.second )
			{
				delete set_elem;
			}
		}
		delete collections;
	}

	bool population_manager::create_new_collection( )
	try{
		bool success = false;
		//This collection shall not exist
		if( collections == nullptr )
		{
			collections =  new collection;
			success = true;
			LOG("population_manager::create_new_collection(): New collection created for the player ",player_name ); 
		}
		else
		{
			LOG_WARN("population_manager::create_new_collection(): Failed since the collection for ", player_name ," already exist!"); 
		}
		return success;
	}catch( std::exception& xa )
	{
		LOG_ERR("population_manager::create_new_collection(): Exception thrown, what: " , xa.what() );
		return false;
	}

	population_entity* population_manager::create_new_population_entity( long max_population, 
			population_class p_class, const economics::expense_and_cost& costs, const economics::revenues_and_profits& revenues )
	try{
		LOG("population_manager::create_new_population_entity(): Entering ");
		population_entity* pop = new population_entity;
		pop->max_population = max_population;
		pop->pop_class = p_class;
		//For each population entity the proper economic unit have to be created
		economics::economic_unit* eu = player_obj->economics->create_economic_unit( economics::salary_class( pop->pop_class ) );
		if( eu )
		{
			//Provide the costs and expenses structure
			eu->set_costs( costs );
			eu->set_revenues( revenues );
			pop->entity_eu = eu;
		}
		else
		{
			LOG_ERR("population_manager::create_new_population_entity(): economic_unit creation failed!!");
			pop->entity_eu = nullptr;
		}
		return pop;
	}catch( std::exception& xa )
	{
		LOG_ERR("population_manager::create_new_population_entity(): Exception thrown, what: " , xa.what() );
		return nullptr;
	}

	//Before adding a population entity, it shall be create with create_new_population_entity.
	//Need to be specified to which city belong this population entity
	bool population_manager::add_population_entity( population_entity* pop , const std::string& city  )
	{
		LOG("population_manager::add_population_entity(): For the player ", player_name );
		if( pop->entity_eu != nullptr )
		{
			pop->entity_eu->set_family_size( &pop->population );
			game_manager::game_manager::get_instance()->get_player_objects( player_name )->economics->add_economic_unit( pop->entity_eu );
		}
		else
		{
			LOG_ERR("population_manager::create_new_population_entity(): economic_unit is not valid...!");
		}
		bool success = false;
		if( collections )
		{
			ELOG("population_manager::add_population_entity(): Collection found, adding a new population_entity to " , player_name );
			//This collection exist, adding the population entity
			collections->access_mutex.lock();

			collections->population.push_back( pop );
			collections->calculate_population();

			collections->access_mutex.unlock();

			//Add this population entity to the proper city
			city_population_set[ city ].push_back( pop );
			ELOG("population_manager::add_population_entity(): The city " , city, " have now ", city_population_set[ city ].size(), " population entities" );

			success = true;
		}
		else
		{
			//This collection does not exist
			LOG_WARN("population_manager::add_population_entity(): Not found the collection for ", player_name );
		}
		return success;
	}

	mlong population_manager::calculate_population()
	{
		total_population = max_population = 0;
		collections->calculate_population();
		total_population = collections->total_population;
		max_population = collections->max_population;
		return total_population;
	}

	inline
	void population_manager::increase_grow_cycle_factor()
	{
		population_grow_cycle_factor += population_grow_cycle_inc;
		if( population_grow_cycle_factor > 1000000 ) //Set a limit and force the counter restart
			population_grow_cycle_factor = 0;
	}

	inline
	double population_manager::get_grow_cycle_factor()
	{
		double value = std::sin( population_grow_cycle_factor );
		increase_grow_cycle_factor();
		//Attenuate the negative effect
		if( value < 0 )
		{
			value *= population_grow_cycle_decrease_attenuation;
		}
		return value;
	}

	//For a given population entity apply the population growing algorithm
	mlong population_manager::calculate_population_delta( )
	{
		LOG("population_manager::calculate_population_delta(): For ", player_name, ", population: ",collections->total_population,", max population: ",collections->max_population ); 
		mlong delta = 0, new_population = 0;

		if( collections->total_population < 15 )
		{
			new_population = collections->total_population + 1;
		}
		else
		{
			/*
			 *Use a simple growing algorithm to increase or decrease the current population
			 */
			delta = ( BASE_GROW_RATION - 1 ) * collections->total_population;
			//The delta will be negative in case of starving citizen
			if( collections->total_population > population_grow_cycle_trigger_threshold )
			{
				//Depending on the growing cycle the population can grow quicly or can even decrease
				long double cycle_factor = get_grow_cycle_factor();
				delta *= cycle_factor;
				ELOG("population_manager::calculate_population_delta(): New delta:", delta, " grow_cycle_factor: ",cycle_factor);
			}
			delta = apply_economic_var_to_delta( delta );
			new_population  = collections->total_population + delta; 
		}
		//Check the boundary
		if( new_population > collections->max_population )
		{
			new_population = collections->max_population; 
		}
		if( new_population < 0 )
		{
			new_population = 0;
		}
		return new_population - collections->total_population;
	}

	mlong population_manager::apply_economic_var_to_delta( mlong delta )
	{
		const economics::economic_variables* eco = player_obj->economics->get_the_economic_variables();
		if( delta > 10 )
		{
			//The delta value depends on some economic factors
			mlong tmp = delta , tmp2;
			if( eco->salary_taxes_equity_perception > 0.7 ) //This very bad level impact negatively the population grow
			{
				delta *= - ( 0.5 + eco->salary_taxes_equity_perception );
			}
			else if( eco->salary_taxes_equity_perception < 0.3 )
			{
				delta *= ( 1.3 - eco->salary_taxes_equity_perception ); // Increase the speed
			}
			tmp2 = delta;
			//Also the amount of starving units impact the growing capacity
			long double starving_on_total_ratio;
			if( collections->total_population > 0 )
			{
				starving_on_total_ratio = ( long double ) eco->amount_of_starving_unit / collections->total_population;
				if( starving_on_total_ratio > STARVING_RATIO_THRESHOLD && delta > 0)
				{
					//Is very high, the population dislike this situation. the delta will be negative
					delta *= -1;
				}
				delta *= ( ( delta > 0 ) ? 1 - starving_on_total_ratio : 1 + starving_on_total_ratio );//If the amount of starving unit is 0, there will be no negative or positive impact
			}
			ELOG("population_manager::apply_economic_var_to_delta(): Delta:",tmp,",TaxEqPerc:",eco->salary_taxes_equity_perception,",NewDelta:",tmp2,",StarvTotalRatio:",starving_on_total_ratio,",FinalDelta:",delta );
		}
		else if( delta < 0 )
		{
			delta *= ( 1 + eco->salary_taxes_equity_perception ); //Increase the depopulation speed
		}
		return delta;
	}

	//This function apply the increase population algorithm to a specific collection,
	//return the amount of unit assigned
	long population_manager::increase_population( long delta, collection* coll )
	{
		//The population is going to increase, apply proportionaly the grow rules on the basis
		//of the amount of population in every population_entity
		long double increase_proportion = ( long double ) delta / ( coll->max_population - coll->total_population );
		ELOG("population_manager::increase_population(): increase_proportion: ",increase_proportion,", free space left: ", coll->max_population - coll->total_population );
		if( increase_proportion > 1 )
		{
			LOG_ERR("population_manager::increase_population(): increase_proportion shall not be higher than 1!");
			increase_proportion = 1;
		}
		mlong pop_entity_increase , assigned_population = 0;
		for( auto elem : coll->population )
		{
			if( !elem->can_grow_more() || elem->get_economic_entity()->is_starving() ) //Do not increase this unit if is starving
				continue;
			pop_entity_increase = std::ceil( elem->left_to_max_population() * increase_proportion );
			ELOG("population_manager::increase_population(): pop_entity_increase: ",pop_entity_increase,", proportional increase calculated: ",elem->left_to_max_population() * increase_proportion );
			if( pop_entity_increase == 0 )
			{
				pop_entity_increase = 1;
			}
			long unasigned_pop = elem->change_population( pop_entity_increase );
			if( unasigned_pop > 0 )
			{
				//TODO What do do?
			}
			assigned_population += ( pop_entity_increase - unasigned_pop );
			if( assigned_population >= delta )
			{
				break;
			}
		}
		return assigned_population;
	}

	//Decrease the population of 'delta' elements, delta shall be > 0
	long population_manager::decrease_population( long delta, collection* coll )
	{
		//The population is going to decrease, apply proportionaly the grow rules on the basis
		//of the amount of population in every population_entity
		long double decrease_proportion = ( long double ) delta / ( coll->total_population );
		ELOG("population_manager::decrease_population(): decrease_proportion: ",decrease_proportion,", Population: ", coll->total_population );
		if( decrease_proportion > 1 )
		{
			LOG_ERR("population_manager::decrease_population(): decrease_proportion shall not be higher than 1!");
			decrease_proportion = 1;
		}
		mlong pop_entity_decrease , removed_population = 0;
		for( auto elem : coll->population )
		{
			if( elem->have_zero_population() )
				continue;
			pop_entity_decrease = std::ceil( elem->get_population() * decrease_proportion );
			ELOG("population_manager::decrease_population(): pop_entity_decrease: ",pop_entity_decrease,", proportional decrease calculated: ",elem->get_population() * decrease_proportion );
			if( pop_entity_decrease == 0 )
			{
				pop_entity_decrease = 1;
			}
			long unasigned_pop = elem->change_population( pop_entity_decrease * -1 );
			if( unasigned_pop > 0 )
			{
				//TODO What do do?
			}
			removed_population += ( pop_entity_decrease - unasigned_pop );
			if( removed_population >= delta ) //All the units were removed properly?
			{
				break;
			}
		}
		return removed_population;
	}

	//This function implements the algorithm which review the population of all the population entities
	//on the basis of some growing algorithm
	mlong population_manager::review_population()
	{
		if( collections == nullptr )
		{
			return 0;
		}
		mlong delta = 0 , total_delta = 0;
		//locking the collection map is needed here
		std::lock_guard< std::mutex > lock ( collections->access_mutex );

		calculate_population();
		//Each collection have its calculation grow ratio calculated separatery,
		//the idea is that different collections can have different grow ratio
		collection* coll = collections;
		delta = calculate_population_delta( );
		LOG("population_manager::review_population(): Delta:",delta,",Total pop:",coll->total_population,",Max pop:",coll->max_population);
		if( delta == 0 )
		{
			return 0;
		}
		//Now we have a global population increase or decrease, this amount shall be divided between the population_entity
		//Actually the delta will be distribuited proportionally between all the population_entity
		if( delta > 0 && ( max_population > total_population ) )
		{
			total_delta = increase_population( delta, coll );
		}
		else if( delta < 0 && ( total_population > 0 ) )
		{
			total_delta = decrease_population( std::abs( delta ) , coll );
		}
		return total_delta;
	}

	mlong population_manager::get_total_population()
	{
		calculate_population();
		return total_population;
	}

	mlong population_manager::get_max_population()
	{
		calculate_population();
		return max_population;
	}

	std::pair< mlong , mlong > population_manager::get_city_population( const std::string& city ) 
	{
		std::pair< mlong , mlong > population;
		population.first = population.second = 0;
		if( city_population_set.find( city ) != city_population_set.end() )
		{
			for( auto elem : city_population_set[ city ] )
			{
				population.first += elem->get_population();
				population.second += elem->get_max_population();
			}
		}
		return std::move( population );
	}

	void population_manager::set_player_game_obj( game_manager::player_game_objects* game_obj )
	{
		player_obj = game_obj;
	}
}

