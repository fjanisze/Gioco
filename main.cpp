#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2


#include "logging/logger.hpp"
#include "game.hpp"
#include <process.h>

#include "buildings.hpp"


void execution_thread_b(void*)
{
	LOG("void execution_thread_b(): Is starting! Will handle the event_manager main loop");
	//Running a separate thread for the event_manager
	events::event_manager::get_instance()->main_loop();

	LOG("void execution_thread_b(): Is quitting!");
}

void execution_thread_c(void*)
{
	LOG("void execution_thread_c(): Is starting!");
	do{
		LOG("void execution_thread_c(): Doing nothing!!");
		Sleep( 200 );
	}while( 1 );
	LOG("void execution_thread_c(): Is quitting!");
}


int main()
{
	console_ui::user_interface ui;

	//Read the building descriptor
	buildings::building_manager buildings;

	//This thread will run the event manager object.
	_beginthread( &execution_thread_b, 0 , nullptr );

	//Creating the game scenario
	game_manager::game_manager::get_instance()->create_test_scenario_1();
	
	LOG("int main(): Starting the execution threads..");

	_beginthread( &execution_thread_c, 0 , nullptr );

	ui.input_loop();

	delete game_manager::game_manager::get_instance();
	return 0;
}
