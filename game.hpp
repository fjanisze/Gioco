#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#ifndef GAME_HPP
#define GAME_HPP

#include "logging/logger.hpp"
#include "mytypes.hpp"
#include "map.hpp"
#include "events.hpp"
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
	class game_manager
	{
		static game_manager* instance;
		game_map::gameplay_map   map; //Here we have the game map
	public:
		static game_manager* get_instance();
		game_manager();
		~game_manager();
	public:
		game_map::gameplay_map* get_the_game_map();
		void create_test_scenario_1();
	};
};


namespace console_ui
{
	
#define WHITE_INTENSE_COLOR  FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY 
#define FOREGROUND_WHITE FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE

	struct colors 
	{
		long color;
		colors( long possible_color ) : color( possible_color )
		{	}
	};

	struct choice_range
	{
		long down_limit,
		     up_limit;
		long choice;
		choice_range( mlong from, mlong to ) : down_limit( from ) , up_limit( to )
		{	}
	
	};
	

	static colors color_fore_white( FOREGROUND_WHITE );
	static colors color_fore_red( FOREGROUND_RED );
	static colors color_fore_iwhite( WHITE_INTENSE_COLOR );
	static colors color_fore_green( FOREGROUND_GREEN );
	static colors color_fore_yellow( FOREGROUND_GREEN | FOREGROUND_RED );

	class user_interface;

	typedef vector< string > vec_of_str;
	typedef list< string > list_of_str;

	struct command_pack
	{
		string command; //The command requested by the user
		vec_of_str parms;//Other parameter provided with this command
		//Constructors and other
		command_pack() = default;
		command_pack& operator=( command_pack&& ) = default;
		command_pack( command_pack&& ) = default;
	};

	std::string format_cur_type( const mlong value );

	//Some c-out/in operator were overloaded to allow string coloring and other stuff
	std::ostream& operator<<( ostream& out_stream , const colors& color );
	std::istream& operator>>( istream& in_stream , command_pack& cmd );
	std::istream& operator>>( istream& in_stream , choice_range& choice );

	using namespace game_map;

	struct command
	{
		string name;
		int (user_interface::*function)( const command_pack& );
	};

	class console_input
	{
	public:
		command_pack command_input();
	};

	class user_interface : public console_input
	{
		std::string human_player;
	public:
		void print_obj_list(const field_coordinate& coord);
		user_interface();
		int execute_a_command( const command_pack& cmd, command cmd_table[] );
		void input_loop();
	public: // Generic operations
		int draw_map( const command_pack& );
		int quit( const command_pack& );
		int help( const command_pack& );
	};

	//List of possible commands
	static command command_list[] = 
	{
		{ "help" , &user_interface::help },
		{ "quit" , &user_interface::quit  },
		{ "drawmap" , &user_interface::draw_map },
		{ "" , nullptr }
	};

}

#endif
