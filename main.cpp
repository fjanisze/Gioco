#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "config.hpp"
#include "logging/logger.hpp"
#include "game.hpp"

#include "buildings.hpp"
#include "ui/ui.hpp"
#include "map/city_map.hpp"

#include "ui/draw.hpp"
using namespace drawing_objects;

#if !COMPILE_TEST_MODE

int main()
{
    log_inst.set_thread_name( "MAIN" );

    game_manager::game_manager* game_manager = game_manager::game_manager::get_instance();
    game_manager->init();

	//Creating the game scenario
	game_manager->create_test_scenario_1();
    game_manager->handle_game();


    log_inst.terminate_logger();
//	delete game_manager::game_manager::get_instance();
	return 0;
}

#endif
