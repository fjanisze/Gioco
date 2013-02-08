#include "cities.hpp"
#include "game.hpp"

namespace city_manager
{
	//A city can be considered as a square of a certain size, the default size
	//of the city 'square' edge is defined by default_city_siz
	const long default_city_size = 5;

	//Some basic building definitions
	const building_descriptor civil_small_house = { 1 , "Small house", "Small mono family house", building_type::habitable, true, false, 350000, 650, 3000,  20, 7 , nullptr , population::population_class::class_0 };
	const building_descriptor civil_medium_house = { 2 , "Medium house", "This is a little bigger house, for up to three families", building_type::habitable, true,false, 1350000, 700, 7500, 35, 20 , nullptr , population::population_class::class_0 };
	const building_descriptor civil_small_appartment = { 3 , "Appartment", "Building with a set of small appartment", building_type::habitable, true,false, 14000000, 720, 200000 , 110 , 300 , nullptr , population::population_class::class_0 };
	const building_descriptor civil_medium_appartment = { 4 , "Medium Appartment", "Building with a set of medium size apparment",building_type::habitable, true, false, 32000000, 550, 500000, 250 , 800 , nullptr , population::population_class::class_0 };

	//Special buildings and offices
	const building_descriptor civil_welfare_administration_office = { 5 , "Welfare administration" , "This construction is needed to make the social poor welfare work",building_type::workplace, false, true, 25000000, 0, 3500000, 220 , 600, new civil_welfare_office_actions, population::population_class::class_1  };
	const building_descriptor civil_small_poor_commercial_building ={ 6 , "Commercial wretch shop" , "Small and poor multistore" , building_type::workplace , true, false , 400000, 0, 35000, 90, 60, new civil_small_poor_commercial_actions, population::population_class::class_0 };

	const building_descriptor* buildings_table [] =
	{
		&civil_small_house,
		&civil_medium_house,
		&civil_small_appartment,
		&civil_medium_appartment,
		&civil_welfare_administration_office,
		nullptr
	};


	////////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for 'city' and his base classes
	//
	//
	///////////////////////////////////////////////////////////////////

	void city_map_management::create_empty_map( long size )
	try{
		LOG("city_map_management::create_empty_map(): Creating a new empty map, size ", size*size );
		city_area* tmp_city_area;
		for( int i = 0 ; i < size*size ; i++ )
		{
			tmp_city_area = new city_area;
			city_map.push_back( tmp_city_area );
		}
	}catch( std::exception& xa )
	{
		LOG_ERR("city_map_management::create_empty_map(): Exception thrown, " , xa.what() );
		throw xa; 
	}

	city_map_management::~city_map_management()
	{
		LOG("city_map_management::~city_map_management(): Destroying the city map");
		for( auto elem : city_map )
		{
			delete elem;
		}
	}
	
	city::city( const string& name , const string& owner ) : city_name( name ), city_owner( owner )
	{
		LOG("city::city(): Creating a city, name '",name,"' owner '",owner,"'");
		create_empty_map();
	}

	economics::currency_type city::get_city_net_revenue()
	{
		economics::currency_type net_revenue = 0;
		auto construction_list = get_construction_list();
		if( construction_list.size() > 0 )
		{
			for( auto elem : construction_list )
			{
				if( elem->population != nullptr )
				{
					net_revenue += elem->population->get_unit_net_revenue();
				}
			}
		}
		return net_revenue;
	}

	////////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for city_manager
	//
	//
	////////////////////////////////////////////////////////////////////
	
	city_manager::city_manager( const std::string& player )
	{
		LOG("city_manager::city_manager(): Creating a new city manager for ", player );
		player_name = player;
	}

	city::~city()
	{
	}

	city* city_manager::create_new_city( const string& name )
	try{
		LOG("city_manager::create_new_city(): Creation of a new city requested, name: " , name, ", for the player ", player_name);

		if( get_city_info( name ) != nullptr )
		{
			LOG_WARN("city_manager::create_new_city(): This city already exist! ");
			return nullptr;
		}

		city* city_info = new city( name, player_name );
		city_info->set_player_objects( game_manager::game_manager::get_instance()->get_player_objects( player_name ) );

		cities.push_back( city_info );

		return city_info;
	}catch( std::exception& xa )
	{
		LOG_ERR("city_manager::create_new_city(): Exception thrown! ", xa.what() );
		return nullptr;
	}

	//Return a pointer to the city object for a given city name
	city* city_manager::get_city_info( const string& name )
	{
		ELOG("city_manager::get_city_info(): Requested for ",name,", amount of cities ",cities.size() );
		city* requested_city = nullptr;
		if( cities.empty() )
		{
			return nullptr;
		}
		for( auto elem : cities )
		{
			if( elem->get_city_name() == name )
			{
				requested_city = elem;
				break;
			}
		}
		return requested_city;
	}

	//Return the list of cities which belong to 'player_name'
	vector< city* >* city_manager::get_the_cities( )
	{
		return &cities;
	}

	//Free the allocated resources
	city_manager::~city_manager()
	{
		for( auto elem : cities )
		{
			delete elem;
		}
	}

	////////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for building_info and building_descriptor
	//
	//
	////////////////////////////////////////////////////////////////////
	
	building_descriptor::~building_descriptor()
	{
		if( actions != nullptr )
		{
			delete actions;
		}
	}

	building_info::building_info() : descriptor( nullptr ), 
		workplace_desc( nullptr )
	{	}

	building_info::~building_info()
	{
		if( descriptor != nullptr )
		{
			delete descriptor;
		}
	}

	building_event_action::building_event_action()
	{
	}

	void civil_welfare_office_actions::construction_completed( game_manager::player_game_objects* player_objects )
	{
		LOG("civil_welfare_office_actions::construction_completed(): Enabling the welfare for ", player_objects->player->get_player_name() );
		player_objects->economics->get_public_walfare_funds()->set_welfare_availability( true );
		job_market::job_entity* job = player_objects->economics->create_new_job_entity( &job_market::civil_office_job_level1 );
		if( job )
		{
			player_objects->economics->register_job_entity( job , civil_welfare_administration_office.population_capacity );
		}
	}
	
	void civil_small_poor_commercial_actions::construction_completed( game_manager::player_game_objects* player_objects )
	{
		//Register a new job in the job market
		job_market::job_entity* job = player_objects->economics->create_new_job_entity( &job_market::civil_scullion_job_level0 );
		if( job )
		{
			player_objects->economics->register_job_entity( job , civil_small_poor_commercial_building.population_capacity );
		}
	}

	////////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for workplace_manager
	//
	//
	////////////////////////////////////////////////////////////////////

	workplace_descriptor* workplace_manager::create_new_workplace( building_info* building )
	try{
		workplace_descriptor* workplace_desc = new workplace_descriptor;
		workplace_desc->employer_name = building->get_name();
		return workplace_desc;
	}catch( std::bad_alloc& xa )
	{
		LOG_ERR("* workplace_manager::create_new_workplace(): Allocation failure");
		return nullptr;
	}

	////////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for construction_management
	//
	//
	////////////////////////////////////////////////////////////////////
	
	construction_management::construction_management()
	{
		LOG("construction_management::construction_management(): New object");
		this_city = static_cast< city* >(this);
	}

	//The function return true if this building was constructed in the city
	bool construction_management::is_the_building_present( const building_descriptor* building ) const
	{
		for( const auto elem : constructions )
		{
			if( elem->descriptor->building_id == building->building_id )
				return true;
		}
		return false;
	}

	//Check if the building 'building' can be builded by 'player'
	int construction_management::is_possible_to_build( const building_descriptor* building ) const
	{
		string player = this_city->get_city_owner();
		LOG("construction_management::is_possible_to_build(): Building '",building->name,"', player '",player,"'");
		int return_value = 0; // By default, is not possible
		economics::economy_manager* eco = player_objects->economics;
		if( eco == nullptr )
		{
			LOG_ERR("construction_management::is_possible_to_build(): 'player' is null for ",player,", that's not good! ");
			return 0;
		}
		//Is possible to build one construction at time.
		if( !is_construction_ongoing() )
		{
			//Is the building already present? If yes, can be multiple building constructed?
			if( building->one_build_per_player )
			{
				//Have to check in all the cities
				return_value = 1;
				for( auto elem : *( player_objects->cities->get_the_cities() ) )
				{
					if( elem->is_the_building_present( building ) )
					{
						return_value = 0; //not possible
						break;
					}
				}
			}
			else if( !building->allowed_multiple_builds && is_the_building_present( building ) )
			{
				return_value = 0;
			}
			else if( eco->player_available_money() >= building->price )//Has the player enough money?
			{
				return_value = 1; 
			}
		}
		if( !return_value )
		{
			ELOG("construction_management::is_possible_to_build(): Not allowed!");
		}
		return return_value;
	}

	void construction_management::set_player_objects( game_manager::player_game_objects* obj )
	{
		player_objects = obj;
	}

	//Return true if a costruction is alrady ongoing
	bool construction_management::is_construction_ongoing() const
	{
		for( auto elem : constructions )
		{
			if( elem->remaining_construction_time > 0 )
				return true;
		}
		return false;
	}

	construction_management::~construction_management()
	{
		for( auto elem : constructions )
		{
			delete elem;
		}
	}

	//The function return false if the construction of the building failed
	bool construction_management::start_building_construction( const building_descriptor* building )
	try{
		LOG("city::start_building_construction(): Requested to build ", building->name );
		//Allocate the required resource for this new building
		building_info* new_building = new building_info;
		new_building->remaining_construction_time = building->construction_time; // The construction has just began
		new_building->population = nullptr;
		new_building->descriptor = building;

		constructions.push_back( new_building );

		if( player_objects->economics->player_apply_cost( building->price ) < 0 )
		{
			LOG_WARN("construction_management::start_building_construction(): ",this_city->get_city_owner()," is not able to pay ",building->price," since have not enought money. Transaction not performed");
		}
		long building_event = events::event_manager::get_instance()->register_event( this );
		if( building_event > 0 )
		{
			if( events::event_manager::get_instance()->start_event_countdown( building_event , 
						&new_building->remaining_construction_time , events::event_execution::execute_oneshot ) )
			{
				//Add this trigger to the events handled by trigger_event
				construction_triggers[ building_event ] = new_building;
			}
			else
			{
				LOG_ERR("construction_management::start_building_construction(): Unable to start the event countdown for ", building_event, " construction: ", building->name );
			}
		}
		else
		{
			LOG_ERR("construction_management::start_building_construction(): Unable to register the event with register_event!");
		}
		
		return true;
	}catch( std::exception& xa )
	{
		LOG_ERR("city::start_building_construction(): Exception thrown! ", xa.what() );
		return false;
	}

	const vector< building_info* >& construction_management::get_construction_list() const
	{
		return constructions;
	}

	void construction_management::construction_completed( building_info* building )
	{
		LOG("construction_management::construction_completed(): Building \"" , building->get_name() , "\"" );
		//fill the expense_and_cost and revenues_and_profits structure
		economics::expense_and_cost costs;
		costs.living_cost = building->descriptor->rental_price;
		costs.player_maintanance_cost = building->descriptor->maintanance_cost;

		economics::revenues_and_profits revenues;
		revenues.revenue_from_rental = building->descriptor->rental_price;

		//Create the population_entity for this construction
		if( building->descriptor->type == building_type::habitable )
		{
			//This is an habitable building, like an appartment 
			building->population = player_objects->population->create_new_population_entity( building->descriptor->population_capacity,
				       	building->descriptor->pop_class, costs, revenues );
			if( building->population != nullptr )
			{
				player_objects->population->add_population_entity( building->population, this_city->get_city_name() );
			}
			else
			{
				LOG_ERR("construction_management::construction_completed(): Failed when creating the population entity.. That's not good ");
			}
		}
		else if ( building->descriptor->type == building_type::workplace )
		{
			building->workplace_desc = create_new_workplace( building );
		}
		finance::currency_type tmp = building->descriptor->price;
		building->building_value = tmp * 0.8; //The value decrease after the construction.. :|

		//Call the construction complete action, if present
		if( building->descriptor->actions != nullptr )
		{
			building->descriptor->actions->construction_completed( player_objects );
		}
	}

	int construction_management::trigger_event( long event_id )
	{
		LOG("construction_management::trigger_event(): Event ID: ", event_id );
		map< long , building_info* >::iterator elem;
		if( ( elem = construction_triggers.find( event_id ) ) != construction_triggers.end() )
		{
			//Is the construction build finished?
			if( (*elem).second->remaining_construction_time.get_time_left() <= 0 )
			{
				//Construction completed
				construction_completed( ((*elem).second) );
			}
			else
			{
				LOG_WARN("construction_management::trigger_event(): Unexpected trigger for the event " , event_id);
			}
		}
		else
		{
			LOG_ERR("construction_management::trigger_event(): Unable to find the event ID " , event_id);
		}
		ELOG("construction_management::trigger_event(): Handling of the event ",event_id," completed!");
		return 0;
	}
}

