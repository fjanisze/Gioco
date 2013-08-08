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
		cities::city_agent* torino_agent = city_manager->create_new_city( "Torino", 144 );

		//Create the cities on the map
		get_the_game_map()->create_a_city_at_random_coord( "Roma" , roma_agent );
		get_the_game_map()->create_a_city_at_random_coord( "Milano" , milano_agent );
		get_the_game_map()->create_a_city_at_random_coord( "Torino" , torino_agent );

	}

	void game_manager::init()
	{
        std::lock_guard< std::mutex > lock(mutex);
	    LOG("game_manager::init(): Initialize the game..");
	    //Read the building descriptor
        buildings = new(nothrow) buildings::building_manager;
        assert( buildings != nullptr );
        //City manager
        city_manager = new cities::city_manager( graphic_ui::game_ui::get_instance()->get_viewport_settings() );
        assert( city_manager != nullptr );
        //Construction manager
        contruction_manager = new(nothrow) constructions::construction_manager;
        assert( contruction_manager != nullptr );
        //Event manager
        events = new(nothrow) events::event_manager;
        assert( events != nullptr );
	}

	void game_manager::handle_game()
	{
	    LOG("game_manager::handle_game(): Starting");
	    //User Interface object
	    game_ui = graphic_ui::game_ui::get_instance();
	    //Populate the map with all the vertex
	    get_the_game_map()->create_vertex_map();
	    game_ui->create_render_window();
	    game_ui->main_loop();
	}

	//Return the 'building_manager' object
	buildings::building_manager* game_manager::get_buildings()
	{
	    return buildings;
	}

	graphic_ui::game_ui* game_manager::get_game_ui()
	{
	    return game_ui;
	}

	constructions::construction_manager* game_manager::get_construction_manager()
	{
	    return contruction_manager;
	}

	events::event_manager* game_manager::get_event_manager()
	{
	    return events;
	}
}

