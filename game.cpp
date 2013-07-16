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

namespace console_ui
{

	std::ostream& operator<<( ostream& out_stream , const colors& color )
	{
		SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), color.color );
		return out_stream;
	}

	std::string format_cur_type( const mlong& value )
	{
		//TODO
		string value_str;
		stringstream s_stream;
		s_stream.precision( 20 );
		s_stream  << value;
		value_str = s_stream.str();
		//Find the decimal point if present.
		short decimal_point;
		bool decimal_point_found = false;
		for( decimal_point = 0 ; decimal_point < value_str.size() ; decimal_point++ )
		{
			if( value_str[ decimal_point ] =='.' )
			{
				value_str = value_str.substr( 0 , decimal_point + 3 );
				decimal_point_found = true;
				break;
			}
		}
		if( !decimal_point_found )
		{
			value_str += ".-";
		}
		short position = decimal_point , comma_index = 0, end_pos = 0;
		if( value < 0 )
		{
			end_pos = 1;
		}
		while( position > end_pos )
		{
			if( comma_index != 0 && ( comma_index % 3 == 0 ) )
			{
				value_str.insert( position , "," );
			}
			++comma_index;
			--position;
		}
		return value_str+"$";
	}

	std::istream& operator>>( istream& in_stream , command_pack& cmd )
	{
		static char input_buffer[ 256 ];
		//Get the entire command line
		in_stream.sync();
		do{
			in_stream.getline( input_buffer, 256 , '\n' );
			if( strlen( input_buffer ) <= 1 )
			{
				LOG_WARN("console_input::command_input(): The input string is too short!");
				cout<<"The provided command is too short, retype: ";
			}
			else break;
		}while( 1 );
		//Parse the line for the single components.
		short begin_word, end_word;
		for( begin_word = end_word = strlen( input_buffer) ; begin_word > 0  ; begin_word-- )
		{
			if( input_buffer[ begin_word ] == ' ' )
			{
				cmd.parms.insert( cmd.parms.begin(), string( input_buffer + begin_word + 1 , input_buffer + end_word ) );
				end_word = begin_word;
			}
		}
		cmd.command = string( input_buffer , input_buffer + end_word );
		LOG("console_input::command_input(): Requested to execute:", cmd.command );
		return in_stream;
	}

	std::istream& operator>>( istream& in_stream , choice_range& choice )
	{
		in_stream.sync();
		do{
			in_stream>>choice.choice;
			if( choice.choice <= choice.up_limit && choice.choice >= choice.down_limit )
			{
				ELOG("std::istream& operator>>(): Player choice, " , choice.choice );
				break;
			}
			else
			{
				cout<<"Invalid choice, try again!: ";
			}
		}while( 1 );
		return in_stream;
	}

	//Return a vector of strings which contain the command requested by the user.
	//The delimiter is \n
	command_pack console_input::command_input()
	{
		command_pack cmd;
		static char input_buffer[ 256 ];
		//Get the entire command line
		cin.sync();
		do{
			cin.getline( input_buffer, 256 , '\n' );
			if( strlen( input_buffer ) <= 1 )
			{
				LOG_WARN("console_input::command_input(): The input string is too short!");
				cout<<"The provided command is too short, retype: ";
			}
			else break;
		}while( 1 );
		//Parse the line for the single components.
		short begin_word, end_word;
		for( begin_word = end_word = strlen( input_buffer) ; begin_word > 0  ; begin_word-- )
		{
			if( input_buffer[ begin_word ] == ' ' )
			{
				cmd.parms.insert( cmd.parms.begin(), string( input_buffer + begin_word + 1 , input_buffer + end_word ) );
				end_word = begin_word;
			}
		}
		cmd.command = string( input_buffer , input_buffer + end_word );
		LOG("console_input::command_input(): Requested to execute:", cmd.command );
		return std::move( cmd );
	}

	user_interface::user_interface() //: game( game_manager_ptr )
	{
	}

	//This function draw the map in the consolle
	int user_interface::draw_map( const command_pack& cmd )
	{
		LOG("user_interface::draw_map(): Drawing the gameplay map");
		auto field_map = game_manager::game_manager::get_instance()->get_the_game_map()->get_fieldmap();
		long map_size = game_manager::game_manager::get_instance()->get_the_game_map()->get_map_size();
		char symbol, last_symbol;
		for( auto it : field_map )
		{
			if( /* it->state ^ not_explored */ 1 ) // TODO
			{
				//Print in different color depending on the type of field
				symbol = it->manager->get_field_symbol();
				if( symbol != last_symbol )
				{
					switch( symbol )
					{
					case 'F':
						cout<<color_fore_green;
						break;
					case 'C':
						cout<<color_fore_red;
						break;
					case '.':

					default:
						cout<<color_fore_white;
						break;
					}
				}
				cout<<symbol;
				last_symbol = symbol;
			}
			else
			{
				cout<<not_discovered_field_symbol;
			}
			if( it->coord.x == ( map_size - 1 ) )
			{
				cout<<endl;
			}
		}
		//Set again the default color
		cout<<color_fore_white;
		cout<<endl;
		return 1;
	}

	//Print the list of objects present on a certain map coordinate
	void user_interface::print_obj_list( const field_coordinate& coord )
	{
		LOG("user_interface::print_obj_list(): Coordinate ",coord.x,",",coord.y);
		long index = game_manager::game_manager::get_instance()->get_the_game_map()->calculate_index( coord );
		auto map =  game_manager::game_manager::get_instance()->get_the_game_map()->get_fieldmap();
		if( index >= 0 )
		{
			map_field* field = map[ index ];
			if( field->manager )
			{
				obj_list_t& obj_list = field->manager->get_obj_list();
				if( obj_list.size() )
				{
					cout<<"Those are the objects present on this field:\n";
					short index = 1;
					for( auto it : obj_list )
					{
						cout<<index<<" - "<<it->name.c_str()<<endl;
						++index;
					}
				}
				else
				{
					cout<<"Nothing visible in this field.. is empty.\n";
				}
			}
			else
			{
				LOG_WARN("user_interface::print_obj_list():field->manager is NULL!! This looks strange" );
				cout<<"Nothing to show..\n";
			}
		}
		else
		{
			cout<<"Not valid coordinates.. this fiels does not exist..\n";
			LOG("user_interface::print_obj_list(): Not valid coordinate provided.." );
		}
	}

	int user_interface::quit( const command_pack& cmd )
	{
		return 0;
	}

	int user_interface::help( const command_pack& cmd )
	{
		cout<<"\nThis is the list of possible commands. \n";
		short i = 0;
		for( auto elem : command_list )
		{
			if( elem.function != nullptr )
			{
				cout<<"  #"<<i<<" "<<elem.name<<endl;
				++i;
			}
		}
		return 1;
	}

	//Main loop for the user interface
	void user_interface::input_loop()
	{
		command_pack command;
		int return_value;
		do{
			LOG("user_interface::input_loop(): Waiting for input");
			cout<<color_fore_iwhite<<"\n[ Main Menu ]";
			cout<<color_fore_white<<" What to do?\n:>";
			command = std::move( command_input() );
			return_value = execute_a_command( command, command_list );
		}while( return_value != 0 );
	}

	int user_interface::execute_a_command( const command_pack& cmd, command cmd_table[] )
	{
		LOG("user_interface::execute_a_command(): Executing \"",cmd.command,"\"");
		bool command_executed = false;
		int return_value = numeric_limits< int >::max();
		//Exist any operation for this command?
		command* elem = &cmd_table[ 0 ];
		for( int i = 0 ; cmd_table[ i ].function != nullptr ; i++, elem = &cmd_table[ i ] )
		{
			if( cmd.command == elem->name )
			{
				//Call the function
				return_value = (this->*(elem->function))( cmd );
				command_executed = true;
				break;
			}
		}
		if( !command_executed )
		{
			cerr<<"Unknow command \'"<<cmd.command<<"\'!\n";
			LOG_WARN("user_interface::execute_a_command(): Cannot execute \'",cmd.command,"\'. The command is unknow");
		}
		return return_value;
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
		citymap::city_agent* roma_agent = city_manager->create_new_city( "Roma", 100 );
		citymap::city_agent* milano_agent = city_manager->create_new_city( "Milano", 100 );

		//Create the cities on the map
		get_the_game_map()->create_a_city_at_random_coord( "Roma" , roma_agent );
		get_the_game_map()->create_a_city_at_random_coord( "Milano" , milano_agent );
	}

	void game_manager::init()
	{
        std::lock_guard< std::mutex > lock(mutex);
	    LOG("game_manager::init(): Initialize the game..");
	    //Read the building descriptor
        buildings = new(nothrow) buildings::building_manager;
        assert( buildings != nullptr );
        //City manager
        city_manager = new citymap::city_manager( graphic_ui::game_ui::get_instance()->get_viewport_settings() );
        assert( city_manager != nullptr );
	}

	void game_manager::handle_game()
	{
        std::lock_guard< std::mutex > lock(mutex);
	    LOG("game_manager::handle_game(): Starting");
	    //User Interface object
	    graphic_ui::game_ui* game_ui = graphic_ui::game_ui::get_instance();
	    //Populate the map with all the vertex
	    get_the_game_map()->create_vertex_map();
	    game_ui->create_render_window();
	    game_ui->main_loop();
	}
}

