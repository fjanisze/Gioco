#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "logging/logger.hpp"
#include "game.hpp"
#include <process.h>

#include "buildings.hpp"
#include "ui.hpp"
#include "city_map.hpp"


void execution_thread_b(void*)
{
	LOG("void execution_thread_b(): Is starting! Will handle the event_manager main loop");
	//Running a separate thread for the event_manager
	events::event_manager::get_instance()->main_loop();

	LOG("void execution_thread_b(): Is quitting!");
}

void graphical_ui_thread(void*)
{
	LOG("void graphical_ui_thread(): Is starting!");

	LOG("void graphical_ui_thread(): Is quitting!");
}


int main()
{


	//This thread will run the event manager object.
	_beginthread( &execution_thread_b, 0 , nullptr );

    game_manager::game_manager* game_manager = game_manager::game_manager::get_instance();
    game_manager->init();

	//Creating the game scenario
	game_manager->create_test_scenario_1();
    game_manager->handle_game();


//	delete game_manager::game_manager::get_instance();
	return 0;
}
