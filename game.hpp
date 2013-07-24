#ifndef GAME_HPP
#define GAME_HPP

#include "logging/logger.hpp"
#include "mytypes.hpp"
#include "map.hpp"
#include "buildings.hpp"
#include "events.hpp"
#include "city_map.hpp"
#include "cities.hpp"
#include "ui.hpp"
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

using namespace std;

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
		std::string get_player_name();
	};
}

namespace game_manager
{

    //Manage the whole game logic
	class game_manager
	{
	    std::mutex mutex;
		static game_manager* instance;
		buildings::building_manager* buildings;
		cities::city_manager* city_manager;
	public:
		static game_manager* get_instance();
		game_manager();
		~game_manager();
	public:
		game_map::game_map* get_the_game_map();
		void create_test_scenario_1();
		void init();
		void handle_game();
	};
};


#endif
