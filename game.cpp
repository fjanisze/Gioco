#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "game.hpp"

namespace player_info
{
	player_manager::player_manager(const string& name , type_of_player player_type )
	try{
		player_info* pl = new player_info;
		pl->name = name;
		pl->player_type = player_type;
		player = pl;
		LOG("player_manager::player_manager(): New player created: ", name.c_str() );
	}catch(exception& xa)
	{
		LOG_ERR("player_manager::player_manager(): Exception thrown, ", xa.what(), ". Not able to recover, killing the application" );
		exit( -1 );
	}

	player_info* player_manager::get_player_info(  )
	{
		return player;
	}

	std::string player_manager::get_player_name()
	{
		return player->name;
	}
}

////////////////////////////////////////////////////////////////////
//
//
//	Follow the implementation for game_manager
//
//
////////////////////////////////////////////////////////////////////

namespace game_manager
{

	game_manager* game_manager::instance = nullptr;

	game_manager* game_manager::get_instance()
	{
		if( instance == nullptr )
		{
			instance = new game_manager;
		}
		return instance;
	}

	game_manager::game_manager()
	{
		LOG("game_manager::game_manager(): New game manager");
	}

	game_manager::~game_manager()
	{
		//TODO:  The memory allocated by the call to get_instance is not freed
		LOG("game_manager::~game_manager(): Game manager destroyed");
	}

	game_map::game_map* game_manager::get_the_game_map()
	{
		return game_map::game_map::get_instance();
	}

	cities::city_manager* game_manager::get_city_manager()
	{
	    return city_manager;
	}

	//Create a test scenario
	void game_manager::create_test_scenario_1()
	{
	    std::lock_guard< std::mutex > lock(mutex);
		LOG("game_manager::create_test_scenario_1(): Creating a test scenario");

        //Generate the random map.
		get_the_game_map()->create_new_map( 50 );
		get_the_game_map()->generate_random_map();

        //Create the cities in the game.
		cities::city_agent* roma_agent = city_manager->create_new_city( "Roma", 100 );
		cities::city_agent* milano_agent = city_manager->create_new_city( "Milano", 100 );
		cities::city_agent* torino_agent = city_manager->create_new_city( "Torino", 100 );

		//Create the cities on the map
		get_the_game_map()->create_a_city_at_random_coord( roma_agent );
		get_the_game_map()->create_a_city_at_random_coord( milano_agent );
		get_the_game_map()->create_a_city_at_random_coord( torino_agent );

	}

	void game_manager::init()
	{
        std::lock_guard< std::mutex > lock(mutex);
	    LOG("game_manager::init(): Initialize the game..");
	    //Read the building descriptor
        construction_manager = new(nothrow) constructions::construction_manager;
        assert( construction_manager != nullptr );
        //City manager
        city_manager = new cities::city_manager( graphic_ui::game_ui::get_instance()->get_game_canvas_settings() );
        assert( city_manager != nullptr );
        //Event manager
        events = new(nothrow) events::event_manager;
        assert( events != nullptr );
        //Create the population manager
        population_manager = new population::population_manager();
        assert( population_manager != nullptr );
        //User Interface object
	    game_ui = graphic_ui::game_ui::get_instance();
	}

	void game_manager::stop()
	{
	    LOG("game_manager::stop(): Cleaning, the first step before the termination.");
	    delete construction_manager;
	    delete city_manager;
	    delete game_ui;
	    delete events;
	    LOG("game_manager::stop(): Everything clean, goodbye!");
	}

	void game_manager::handle_game()
	try{
	    LOG("game_manager::handle_game(): Starting");
	    //Populate the map with all the vertex
	    get_the_game_map()->create_vertex_map();
	    // game_ui->create_render_window(); TODO
        drawing_objects::drawing_facility* draw = drawing_objects::drawing_facility::get_instance();
        draw->start();
	    game_ui->main_loop();
	    draw->terminate();
	    LOG("game_manager::handle_game(): Out of the main loop.");
	    stop();
	}catch( std::exception& xa )
	{
	    LOG_ERR("game_manager::handle_game(): Exception caught, what: " , xa.what() );
	    return;
	}

	//Return the 'construction_manager' object
	constructions::construction_manager* game_manager::get_buildings()
	{
	    return construction_manager;
	}

	graphic_ui::game_ui* game_manager::get_game_ui()
	{
	    return game_ui;
	}

	constructions::construction_manager* game_manager::get_construction_manager()
	{
	    return construction_manager;
	}

	events::event_manager* game_manager::get_event_manager()
	{
	    return events;
	}

	////////////////////////////////////////////////////////////////////
    //
    //
    //	Follow the implementation for specific 'use case'
    //
    //
    ////////////////////////////////////////////////////////////////////

    //The user want to start a new construction
    bool game_manager::user_want_start_construction( long construction_id, long city_id, long field_id )
    {
        LOG("game_manager::user_want_start_construction(): Construction ID:",construction_id,",city ID:",city_id,",field ID:",field_id);
        //Get the building.
        constructions::construction_t* constr_obj = construction_manager->get_construction_obj( construction_id );
        if( constr_obj == nullptr )
        {
            LOG_ERR("game_manager::user_want_start_construction(): Unable to retrieve the constrution with ID:", construction_id );
            return false;
        }
        //Get the city
        cities::city_agent* city_agent = city_manager->get_city_agent( city_id );
        ELOG("game_manager::user_want_start_construction(): City name: ",city_agent->get_city_info()->name );
        if( city_agent == nullptr )
        {
            LOG_ERR("game_manager::user_want_start_construction(): Unable to retrieve the city agent for the city with ID:", city_id );
            return false;
        }
        citymap::citymap_t* citymap = city_agent->get_city_map();
        if( citymap == nullptr )
        {
            LOG_ERR("game_manager::user_want_start_construction(): Unable to retrieve the citymap for the city with ID:", city_id );
            return false;
        }
        //Ok now we have all the object ready to be used.
        //The construction process is pretty simple, the construction_t is just added on the field if no other constructions are present
        citymap->set_construction( field_id, constr_obj );
        LOG("game_manager::user_want_start_construction(): Construction completed");
        return true;
    }
}























