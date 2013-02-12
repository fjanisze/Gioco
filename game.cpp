#include "game.hpp"


namespace game_objects
{

	bool is_a_terrain_object(const object_descriptor& obj)
	{
		if( obj.obj_id < 10 ) 
			return true;
		return false;
	}
}


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

namespace game_map
{

	map_field::map_field()
	{
		manager = nullptr;
	}

	map_field::~map_field()
	{
		if( manager )
		{
			delete manager;
		}
	}

	////////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for map_calculation
	//
	//
	////////////////////////////////////////////////////////////////////

	//Calculate the absolute distance between two points on the map
	double map_calculation::distance( const fc& point1, const fc& point2 )
	{
		long t1, t2;
		//Simple pythagorean theorem implementation
		t1 = point1.x - point2.x;
		if( t1 != 0 )
		{
			t1 = pow( abs( t1 ) , 2 );
		}
		t2 = point1.y - point2.y;
		if( t2 != 0 )
		{
			t2 = pow( abs( t2 ) , 2 );
		}
		return sqrt( t1 + t2 );
	}

	inline
	bool map_calculation::are_coordinates_equal( const fc& point1, const fc& point2 )
	{
		return (point1.x == point2.x) && (point1.y == point2.y);
	}

	//Find the closest field to 'origin' from 'other_fields'
	//The return value is the index in 'other_fields' of the closest field
	long map_calculation::first_closest_field( const fc& origin, const vector< fc >& other_fields )
	{
		ELOG("map_calculation::first_closest_field(): Looking for the closest field to ",origin.x,":",origin.y,", other_field size: ",other_fields.size() );
		if( other_fields.empty() )
		{
			ELOG("map_calculation::first_closest_field(): Calculation not possible, other_fields has too few elements");
			return -1;
		}
		long closest_field_index = 0;
		long double lower_distance = numeric_limits< long double >::max(), tmp_distance = 0.0;
		for( long index = 0 ; index < other_fields.size() ; index++ )
		{
			if( are_coordinates_equal( origin, other_fields[ index ] ) )
					continue;

			tmp_distance = distance( origin, other_fields[ index ] );
			//ELOG("map_calculation::first_closest_field(): The distance between ",origin.x,":",origin.y," and ",other_fields[ index ].x,":", other_fields [ index ].y," is ",tmp_distance );
			if( lower_distance > tmp_distance )
			{
				lower_distance = tmp_distance;
				closest_field_index = index;
			}
		}
		return closest_field_index;
	}

	//Return a list of points which delimitate the perimeter of 'origin' at a distance 'range'
	list< field_coordinate > map_calculation::get_field_perimeter( const field_coordinate& origin, int range )
	{
		//ELOG("map_calculation::get_field_perimeter(): Looking for the perimeter of ",origin.x,"/",origin.y," at a range of ", range );
		field_coordinate fc;
		int square_l;
		list< field_coordinate > points;
		//We are going to build the list of points delimitating the square perimeter:
		int tmp_x , tmp_y;
		square_l = range * 2 + 1;
		fc.x = origin.x + range;
		tmp_y = origin.y + ( ( square_l - 1 ) / 2  );
		do{
			for( int i = origin.y - ( ( square_l - 1 ) / 2  ) ; i < tmp_y ; i++ )
			{
				fc.y = i;
				points.push_back( fc );
			}
			if( fc.x == origin.x - range )
			{
				break; //Quit the loop if all the coord were checked
			}
			fc.x = origin.x - range;
		}while( 1 );
		//Do the same calculation for the y coordinates
		fc.y = origin.y + range;
		tmp_x = origin.x + ( ( square_l - 1 ) / 2  );
		do{
			for( int i = origin.x - ( ( square_l - 1 ) / 2  ) ; i < tmp_x ; i++ )
			{
				fc.x = i;
				points.push_back( fc );
			}
			if( fc.y == origin.y - range )
			{
				break; //Quit the loop if all the coord were checked
			}
			fc.y = origin.y - range;
		}while( 1 );
		//ELOG("map_calculation::get_field_perimeter(): Amount of points found: ", points.size() );
		return std::move( points );
	}

	//In the way from 'origin' to 'dest' this function find the next field on the way
	field_coordinate map_calculation::path_find_next_field( const fc& origin, const fc& dest )
	{
		ELOG("map_calculation::path_find_next_field(): origin ",origin.x,":",origin.y,", dest ",dest.x,":",dest.y);
		vector< field_coordinate > near_fields;
		long diff_x = origin.x - dest.x,
		     diff_y = origin.y - dest.y;

		if( diff_x != 0 )
		{
			near_fields.push_back( fc( diff_x > 0 ? origin.x - 1 : origin.x + 1 , origin.y ) );
		}
		if( diff_y != 0 )
		{
			near_fields.push_back( fc( origin.x , diff_y > 0 ? origin.y - 1 : origin.y + 1 ) );
		}
		long index = first_closest_field( dest , near_fields );
		if( index < 0 )
		{
			ELOG("map_calculation::path_find_next_field(): No closest field found!!!" );
			return fc( -1, -1 );
		}
		ELOG("map_calculation::path_find_next_field(): Field found ",near_fields[ index ].x,":",near_fields[ index ].y);
		return near_fields[ index ];
	}

	////////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for gameplay_map
	//
	//
	////////////////////////////////////////////////////////////////////

	gameplay_map::gameplay_map()
	{
	}

	void gameplay_map::create_new_map( long size )
	try{
		LOG( "gameplay_map::create_new_map(): Creating new map, size=", size );
		map.reserve( size * size );
		map_field* mp_field;
		map_size = size;
		num_of_fields = size * size;
		long id = 0, x_pos = 0, y_pos = 0;
		
		for( long i = 0 ; i < size*size ; i++ )
		{
			mp_field = new map_field;
			mp_field->manager = new field_manager; 

			mp_field->field_id = ++id;
			mp_field->state = not_owned | not_explored;
			mp_field->value = terrain_grass.price; //The grass will be the basic terrain

			mp_field->manager->add_object( &terrain_grass ); 

			//setting coordinates
			mp_field->coord.x = x_pos;
			mp_field->coord.y = y_pos;
			map.push_back( mp_field );

			if( ++x_pos >= size )
			{
				x_pos = 0;
				++y_pos;
			}
		}
		LOG("gameplay_map::create_new_map(): New map of ",size*size," size" );
	}catch(std::exception& xa)
	{
		LOG_ERR("gameplay_map::create_new_map(): Fatal exception: ", xa.what() );
		exit( -1 );
	}

	gameplay_map::~gameplay_map()
	{
		for( auto it : map )
		{
			delete it;
		}
	}

	field_manager* gameplay_map::add_obj_to_field(const field_coordinate& coord, const object_descriptor* obj)
	{
		LOG("gameplay_map::add_obj_to_field(): Parm: ", coord.x, ":", coord.y," -> ", obj->name.c_str());
		//Get map index
		long index = calculate_index( coord );
		if( index >= 0 )
		{
			map[ index ]->manager->add_object( obj );
		}
	}

	inline
	long gameplay_map::calculate_index( const field_coordinate& coord)
	{
		long index = coord.y * map_size + coord.x;
		if( index >=0 && index < map.size() )
		{
			return index;
		}
		return -1;
	}

	void gameplay_map::generate_random_map()
	{
		const double tree_factor = 0.5;
		const short amount_of_crop = 100;
		srand( time( nullptr ) );

		long amount_of_tree = num_of_fields * tree_factor;
		LOG("gameplay_map::generate_random_map(): Generating a random map, # of crop: ",amount_of_crop," amount of trees: ",amount_of_tree);
		//Generate a random distrution of map coordinates
		vector< field_coordinate > crop_coord; 
		for( short i = 0 ; i < amount_of_crop ; i++ )
		{
			crop_coord.push_back( field_coordinate( rand() % map_size, rand() % map_size ) );
			add_obj_to_field( crop_coord[i] , &terrain_forest );
		}
		//Fill all the tree terraing
		field_coordinate current_field, next_field , tmp_coord;
		long nearest_field,
		     amount_of_trees_per_crop = amount_of_tree / ( amount_of_crop + 1 );
		for( long i = 0 , j ; i < crop_coord.size() ; i++ )
		{
			current_field = crop_coord.back();
			crop_coord.pop_back(); 
			//Look for the nearest field, the idea is that the forest
			//will grow connecting one crop field to another
			nearest_field = first_closest_field( current_field, crop_coord );
			if( nearest_field >= 0 )
			{
				//We found something, now populate the fields between those two points
				for( j = 0 ; j < amount_of_trees_per_crop ; j++)
				{
					if( !are_coordinates_equal( current_field, crop_coord[ nearest_field ] ) )
					{
						next_field = path_find_next_field( current_field, crop_coord[ nearest_field ] );
					}
					else
					{
						break;
					}

					if( next_field.x < 0 )
					{
						//Path over
						break;
					}

					add_obj_to_field( next_field , &terrain_forest );
					current_field = next_field;
				}
				ELOG("gameplay_map::generate_random_map(): No more closest fields found, next_field.x < 0, amount of trees inserted ",j, " max amount of trees for this crop ",amount_of_trees_per_crop );
				if( j < amount_of_trees_per_crop )
				{
					//More items have to be added
					LOG("gameplay_map::generate_random_map(): Adding more trees near the crop.. " , amount_of_trees_per_crop - j , " remaining" );
					while( j < amount_of_trees_per_crop  )
					{
						tmp_coord = find_closest_field_of_type( current_field , &terrain_grass );
						if( are_coord_valid( tmp_coord ) )
						{
							add_obj_to_field( tmp_coord, &terrain_forest );
						}
						else break;
						++j;
					}
				}
			}
			else
			{
				//Nothing can be found, populate just the fields around the crop point
				//TODO
			}
			//Insert again the removed element, this time in the front of the vector
			//This guarantee that will be not used again as 'current_field'
			crop_coord.insert( crop_coord.begin(),  current_field );
		}
	}

	bool gameplay_map::are_coord_valid( const field_coordinate& coord )
	{
		if( coord.x < 0 || coord.y < 0 )
			return false;
		if( coord.x > map_size || coord.y > map_size )
			return false;
		return true;
	}

	//Find a random field of type 'type'
	field_coordinate gameplay_map::find_random_field( const object_descriptor& type )
	{
		LOG("field_coordinate gameplay_map::find_random_field(): obj ",type.name);
		field_coordinate fc;
		srand( time( nullptr ) );
		long attempt_limit = map_size * 2 - 1;
		do{
			fc.x = rand() % map_size;
			fc.y = rand() % map_size;
			if( check_field_type_presence( fc, type ) )
			{
				break; //Got
			}
		}while( attempt_limit-- > 0 );
		if( attempt_limit <= 0 )
		{
			LOG_WARN("field_coordinate gameplay_map(): Was not able to find the requested field!!!");
			set_invalid_coord( fc );
		}
		return fc;
	}

	//Return true if the type object in 'coord' is like expected in 'expected obj'
	bool gameplay_map::check_field_type_presence( const field_coordinate& coord,
						const object_descriptor& expected_obj )
	{
		long index = calculate_index( coord );
		if( index >= 0 )
		{
			obj_list_t& obj_list = map[ index ]->manager->get_obj_list();
			for( auto it : obj_list )
			{
				if( it->obj_id == expected_obj.obj_id )
				{
					//Ok, this object is present
					return true;
				}
			}
		}
		return false;
	}

	//Set invalid coordinate values in 'coord'
	void gameplay_map::set_invalid_coord( field_coordinate& coord )
	{
		coord.x = map_size;
		coord.y = map_size;
	}

	//The next function find the closest field which is not of 'obj' type
	field_coordinate gameplay_map::find_closest_field_of_type( const field_coordinate& origin, const object_descriptor* type )
	{
		//ELOG("gameplay_map::find_closest_field_of_type(): Looking for the closest obj ",type->name," to ",origin.x,":",origin.y);
		field_coordinate fc;
		set_invalid_coord( fc );
		int range = 1;
		list< field_coordinate > points;
		do{
			points = std::move( get_field_perimeter( origin, range ) );
			//Look for 'type' in the list of point provided
			for( auto elem : points )
			{
				if( check_field_type_presence( elem , *type ) )
				{
					//ELOG("gameplay_map::find_closest_field_of_type(): Field found at coord ",elem.x,":",elem.y);
					return elem;
				}
			}
			++range;
		}while( range < 10 ); //Default range set to 10!!
		ELOG_WARN("gameplay_map::find_closest_field_of_type(): Using a default range of 10, no field was found!!! ");
		return fc;
	}

	void gameplay_map::make_all_map_explored()
	{
		LOG("gameplay_map::make_all_map_explored(): Making all map visible to the player");
		for( auto it : map )
		{
			it->state &= ( 0xF & not_explored );
		}
	}

	//The next function create a city field in a specific point of the map
	field_manager* gameplay_map::create_a_city_at_random_coord( const string& name )
	{ 
		ELOG("gameplay_map::create_a_city_at_random_coord(): Creating \'", name  );
		field_manager* field = nullptr;
		//First of all, the proper object will be addeo
		field_coordinate city_coord = find_random_field( terrain_grass );
		if( are_coord_valid( city_coord ) )
		{
			field = add_obj_to_field( city_coord , &terrain_city );
		}
		else
		{
			LOG_ERR("gameplay_map::create_a_city_at_random_coord(): Unable to create the city!! Not found any valid random coordinate");
		}
		return field;
	}

	////////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for field_manager
	//
	//
	////////////////////////////////////////////////////////////////////

	field_manager::field_manager()
	{
		//Default object is of a 'land' type
		object_descriptor* od =  create_new_obj_descriptor();
		*od = base_land;
		symbol = base_land.symbol;

		obj_list.push_back( od );	
	}

	field_manager::~field_manager()
	{
		for( auto it : obj_list )
		{
			delete it;
		}
	}

	object_descriptor* field_manager::create_new_obj_descriptor()
	try{
		object_descriptor* obj_d = new object_descriptor;
		return obj_d;
	}catch(exception& xa)
	{
		LOG_ERR("field_manager::create_new_obj_descriptor(): Exeption catched: ", xa.what() );
		exit( -1 );
	}

	bool field_manager::add_object(const object_descriptor* obj)
	{	
		if( obj )
		{
			object_descriptor* obj_d = create_new_obj_descriptor();
			*obj_d = *obj;

			//If obj_id is lower than 10, this is a terrain type which reppresent the field as symbol
			if( game_objects::is_a_terrain_object( *obj_d ) )
			{
				//ELOG("field_manager::add_object(): Setting new symbol for the field, symbol:'",obj_d->symbol,"' Name: ",obj_d->name );
				symbol = obj_d->symbol; //new symbol for this field
				//Remove other terrain objects, only one terrain at once is allowed in a field
				for( auto it = begin( obj_list ) ; it != end( obj_list ) ; it++ )
				{
					if( game_objects::is_a_terrain_object( **it ) )
					{
						//ELOG("field_manager::add_object(): Only one terrain object allowed per field, removing an existing terrain ");
						obj_list.erase( it );
						break;
					}
				}
			}
			//Add the object to the list
			obj_list.push_back( obj_d );
		}
	}

	obj_list_t& field_manager::get_obj_list()
	{
		return obj_list;
	}

	char field_manager::get_field_symbol()
	{
		return symbol;	
	}

}


namespace console_ui
{

	std::ostream& operator<<( ostream& out_stream , const colors& color )
	{
		SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), color.color );
		return out_stream;
	}

	std::string format_cur_type( const finance::currency_type& value )
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

	//List the cities which belong to the human player
	int user_interface::list_player_cities( const command_pack& cmd )
	{
		LOG("user_interface::list_player_cities(): Listing the cities");
		string human_player = game_manager::game_manager::get_instance()->get_human_player_name();
		if( !human_player.empty() )
		{
			vector< city_manager::city* >* cities = game_manager::game_manager::get_instance()->get_player_objects( human_player )->cities->get_the_cities();
			if( !cities->empty() )
			{
				short index = 1;
				cout<<"The cities which belong to "<<human_player<<" are:\n";
				for( auto elem : *cities )
				{
					cout<<" #"<<index++<<" - "<<elem->get_city_name()<<endl;
				}
			}
			else
			{
				LOG("user_interface::list_player_cities(): No cities belong to the human player");
				cout<<"No cities belong to the human player ( "<<human_player<<" )!\n";
			}
		}
		else
		{
			LOG_ERR("user_interface::list_player_cities(): Unable to retrive the human player name");
		}
		return 1;
	}

	int user_interface::enter_city( const command_pack& cmd )
	{
		string city_name;
		if( cmd.parms.empty() )
		{
			cout<<"\nWhich city? :";
			cin>>city_name;
		}
		else
		{
			city_name = cmd.parms.back();
		}
		std::string human_player = game_manager::game_manager::get_instance()->get_human_player_name();
		city_manager::city* current_city = game_manager::game_manager::get_instance()->get_player_objects( human_player )->cities->get_city_info( city_name );
		if( current_city != nullptr )
		{
			city_manager_loop( current_city );
		}
		else
		{
			LOG("user_interface::enter_city(): The requested city " , city_name , " does not exist" );
			cout<<"This city "<<city_name<<" does not exist!\n";
		}
		return 1;
	}

	//This just print a screen with the information on the player wealth
	int user_interface::wallet( const command_pack& cmd )
	{
		std::string human_player = game_manager::game_manager::get_instance()->get_human_player_name();
		economics::economy_manager* eco = game_manager::game_manager::get_instance()->get_player_objects( human_player )->economics;
		finance::currency_type player_cash = eco->player_available_money();
		cout<<"You bank account saldo is: "<<color_fore_green<<format_cur_type( player_cash )<<color_fore_white<<endl;
		//Print information about the player funds
		if( eco->get_public_walfare_funds()->is_welfare_available() )
		{
			finance::currency_type poor_welfare = eco->get_public_walfare_funds()->get_poor_subsidies_funds();

			finance::currency_type poor_welfare_total_distributed = eco->get_public_walfare_funds()->get_poor_subsidies_total_funding();
			cout<<"Poor subsidies fund: "<<color_fore_green<<format_cur_type( poor_welfare )<<color_fore_white<<", money distributed: "<<color_fore_green<<format_cur_type( poor_welfare_total_distributed )<<color_fore_white<<endl;
		}
		return 1;
	}

	int user_interface::population_info( const command_pack& cmd )
	{
		population::population_manager* pop_mng_inst = game_obj->population;
		economics::economy_manager* eco = game_obj->economics;
		mlong tot_pop = pop_mng_inst->get_total_population(),
			max_pop = pop_mng_inst->get_max_population();
		
		cout<<"Total population: "<<color_fore_yellow<<tot_pop<<color_fore_white<<", Max population: "<<max_pop<<endl<<endl;
	
		auto cities = game_obj->cities->get_the_cities(); 
		economics::currency_type cities_net_revenue = 0, city_revenue = 0;
		for( auto elem : *cities )
		{
			auto city_population = pop_mng_inst->get_city_population( elem->get_city_name() );
			cout<<"-> "<<color_fore_yellow<<elem->get_city_name()<<color_fore_white<<" Population: "<<color_fore_yellow<<city_population.first<<color_fore_white<<" max population: "<<city_population.second;
			city_revenue = elem->get_city_net_revenue();
			cities_net_revenue += city_revenue;
			cout<<", Revenue: "<<color_fore_green<<format_cur_type( city_revenue )<<color_fore_white<<std::endl;
		}

		const economics::economic_variables* var = eco->get_the_economic_variables();
		mlong starving_unit = var->amount_of_starving_unit;
		if( starving_unit > 0 )
		{
			cout<<"-> Amount of starving persons: "<<color_fore_yellow<<starving_unit<<color_fore_white<<endl;
		}
		long poor_people = eco->get_public_walfare_funds()->get_poor_subsidies_counter();
		if( poor_people > 0 )
		{
			cout<<"-> Amount of persons accessing the poor welfare funds: "<<poor_people<<endl;
		}
		cout<<"Cities net revenue: "<<color_fore_green<<format_cur_type( cities_net_revenue )<<color_fore_white<<"$\n";
		return 1;
	}

	//Taxes setting and information
	int user_interface::tax_manager( const command_pack& cmd )
	{
		if( cmd.parms.empty() )
		{
			//Just show the taxes information
			const economics::economic_variables* eco_var = game_obj->economics->get_the_economic_variables();
			cout<<"\nTax level per salary class:\n";
			for( short i = 0 ; i < 6 ; i++ )
			{
				cout<<"->"<<color_fore_yellow<<" Level "<<i<<", taxes: "<<eco_var->salary_taxes[ i ]<<"%"<<color_fore_white<<endl;
			}
		}
		else if( cmd.parms.size() == 2 )
		{
			//The user want to change the salary level
			short salary_level = atoi( cmd.parms[0].c_str() );
			double new_level = atoi( cmd.parms[1].c_str() );
			new_level /= 100;
			if( ( salary_level < 0 || salary_level > 5 ) ||
				( new_level < 0 || new_level > 100 ) )
			{
				cout<<color_fore_red<<"Not valid parameter, salary_level:"<<salary_level<<", texes:"<<new_level<<color_fore_white<<endl;
			}
			else
			{
				game_obj->economics->set_salary_tax_level( economics::salary_class( salary_level ), new_level );
				cout<<color_fore_green<<"New tax level: "<<new_level<<color_fore_white<<endl;
			}
		}
		return 1;
	}

	//Main loop for the user interface
	void user_interface::input_loop()
	{
		command_pack command;
		game_mng_inst = game_manager::game_manager::get_instance();
		human_player = game_mng_inst->get_human_player_name();
		game_obj = game_mng_inst->get_player_objects( human_player );
		int return_value;
		do{
			LOG("user_interface::input_loop(): Waiting for input");
			cout<<color_fore_iwhite<<"\n[ Main Menu ]";
			cout<<color_fore_white<<" What to do?\n:>";
			command = std::move( command_input() );
			return_value = execute_a_command( command, command_list );
		}while( return_value != 0 );
	}

	void user_interface::city_manager_loop( city_manager::city* ct )
	{
		string city_name = ct->get_city_name();
		set_managed_city( ct );
		LOG("user_interface::city_manager_loop(): The user requested to manage ",city_name," city!");
		cout<<"Hello "<<game_manager::game_manager::get_instance()->get_human_player_name()<<" you're in "<<city_name<<" city administration panel!\n";
		command_pack cmd;
		int return_value = numeric_limits< int >::max();
		do{
			cout<<color_fore_iwhite<<"\n[ City Manager ]";
			cout<<color_fore_white<<" What to do?\n:>";
			cmd = std::move( command_input() );
			if( !cmd.command.empty() )
			{
				return_value = execute_a_command( cmd , city_manager_command_list );
			}
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

	////////////////////////////////////////////////////////////////////
	//
	//
	//	Follow the implementation for city_management_ui
	//
	//
	///////////////////////////////////////////////////////////////////

	inline
	void city_management_ui::set_managed_city( city_manager::city* current_city )
	{
		managed_city = current_city;
	}
	
	//Print the help information for the city management panel
	int city_management_ui::cm_help( const command_pack& )
	{
		cout<<"List of possible commands:\n";
		short i = 0;
		for( auto elem : city_manager_command_list )
		{
			if( elem.function != nullptr )
			{
				cout<<"  #"<<i<<" "<<elem.name<<endl;
				++i;
			}
		}
	}

	int city_management_ui::cm_quit( const command_pack& )
	{
		return 0;
	}

	int city_management_ui::cm_list_city_buildings( const command_pack& parm )
	{
		LOG("city_management_ui::cm_list_city_buildings(): For " , managed_city->get_city_name() );
		auto constructions = managed_city->get_construction_list();
		if(! constructions.empty() )
		{
			short index = 1;
			for( const auto* elem : constructions )
			{
				cout<<" #"<<index++<<" - "<<elem->get_name();
				if( elem->remaining_construction_time.get_time_left() > 0 )
				{
					cout<<" - "<<color_fore_yellow<<"( Under construction, remaining time: "<<elem->remaining_construction_time.get_time_left()<<" )"<<color_fore_white;
				}
				else
				{
					if( elem->descriptor->type == city_manager::building_type::habitable )
					{
						cout<<" - Population: "<<elem->population->get_population()<<" , value:  "<<format_cur_type( elem->building_value )<<", ";
						cout<<"Revenue: "<<format_cur_type( elem->population->get_unit_net_revenue() );
					}
					else
					{
						cout<<" - Workplaces: "<<elem->workplace_desc->job->amount_of_free_workplaces;
					}
				}
				cout<<endl;
			}
		}else
		{
			cout<<"No costructions in this city!\n";
		}
		return 1;
	}

	int city_management_ui::cm_build_construction( const command_pack& )
	{
		LOG("city_management_ui::cm_build_construction(): Entering");
		//Print the buildings the user can build
		cout<<"which building you wish to build?\n";
		short i;
		vector< bool > building_possibilities;
		for( i = 0 ; city_manager::buildings_table[ i ] != nullptr ; i++ )
		{
			cout<<" #"<<i + 1<<" - "<<city_manager::buildings_table[ i ]->name<<" - ";
			if( ! managed_city->is_possible_to_build( city_manager::buildings_table[ i ] ) )
			{
				cout<<color_fore_red<<" ( You cannot build this construction )";
				building_possibilities.push_back( false );
			}
			else
			{
				cout.precision(10);
				cout<<color_fore_green<<" This construction cost "<<format_cur_type( city_manager::buildings_table[ i ]->price );
				building_possibilities.push_back( true );
			}
			cout<<color_fore_white<<endl; 
		}
		cout<<" #"<<i + 1<<" - Quit\n";
		cout<<"Make your choice [ 1 - "<< i <<" ]:";
		choice_range choice( 1 , i );
		cin>>choice;
		--choice.choice;
		if( choice.choice == i )
		{
			return 1;
		}
		//Start to build
		if(! building_possibilities[ choice.choice ] )
		{
			cout<<color_fore_red<<"You cannot build this construction.."<<color_fore_white<<endl;
		}
		else
		{
			if( managed_city->start_building_construction( city_manager::buildings_table[ choice.choice ] ) )
			{
				cout<<"Starting to build \'"<<city_manager::buildings_table[ choice.choice ]->name<<"\'..\n";
			}
			else
			{
				LOG_WARN("city_management_ui::cm_build_construction(): Unable to start the construction, start_building_construction returned false");
				cout<<"Impossible to build..\n";
			}
		}
		return 1;
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

	player_game_objects* game_manager::get_player_objects( const std::string& player )
	{
		player_game_objects* obj = nullptr;
		if( player_objects.find( player ) != player_objects.end() )
		{
			obj = player_objects[ player ];
		}
		return obj;
	}

	player_game_objects* game_manager::create_new_player( const std::string& player, player_info::type_of_player player_type )
	try{
		LOG("game_manager::create_new_player(): Creating a new player, name \"",player,"\"");
		player_game_objects* player_obj = nullptr;
		if( player_objects.find( player ) == player_objects.end() )
		{
			player_obj = new player_game_objects;
			//Create all the player objects
			player_obj->player = new player_info::player_manager( player , player_type );
			player_obj->cities = new city_manager::city_manager( player );
			player_obj->population = new population::population_manager( player );
			player_obj->economics = new economics::economy_manager( player );

			player_obj->population->set_player_game_obj( player_obj );
			player_objects[ player ] = player_obj;
		}
		else
		{
			LOG_WARN("game_manager::create_new_player(): Cannot create the player, since ",player," already exist");
		}
		return player_obj;
	}catch( std::exception& xa )
	{
		LOG_ERR("game_manager::create_new_player: Exception thrown, ", xa.what() );
		return nullptr;
	}

	std::string game_manager::get_human_player_name()
	{
		for( auto elem : player_objects )
		{
			if( elem.second->player->get_player_info()->player_type == player_info::type_of_player::human_player )
			{
				return elem.second->player->get_player_info()->name;
			}
		}
		return "";
	}

	//The test scenario is used for testing purpose
	void game_manager::create_test_scenario()
	{
		LOG("game_manager::create_test_scenario(): New test scenario");
		//Create and add a player!
		create_new_player("Filip", player_info::type_of_player::human_player );

		map.create_new_map( 50 );
		map.generate_random_map();

		player_game_objects* player_obj = get_player_objects( "Filip" );
		if( player_obj == nullptr )
		{
			LOG_ERR("game_manager::create_test_scenario(): Unable to get the player_game_object structure");
			exit( -1 );
		}

		//Increase the wallet size
		LOG("game_manager::create_test_scenario(): Adding Money ");
		player_obj->economics->player_add_cash( 5000000000 );

		//Create one city for 'Filip'
		LOG("game_manager::create_test_scenario(): Creating a city in the map (Roma)");
		map.create_a_city_at_random_coord( "Roma" );
		LOG("game_manager::create_test_scenario(): Creating a city in city_manager (Roma)");
		player_obj->cities->create_new_city( "Roma" );

		LOG("game_manager::create_test_scenario(): Creating a city in the map (Milano)");
		map.create_a_city_at_random_coord( "Milano" );
		LOG("game_manager::create_test_scenario(): Creating a city in city_manager (Milano)");
		player_obj->cities->create_new_city( "Milano" );

		//Add a couple of buildings for Filip
		city_manager::city* city_milano = player_obj->cities->get_city_info("Milano");
		city_manager::city* city_roma = player_obj->cities->get_city_info("Roma");


		city_milano->start_building_construction( &city_manager::civil_medium_house );
		city_milano->start_building_construction( &city_manager::civil_small_house );
		city_milano->start_building_construction( &city_manager::civil_small_poor_commercial_building );
	/*	city_milano->start_building_construction( &city_manager::civil_small_appartment );
		city_milano->start_building_construction( &city_manager::civil_medium_appartment );
		city_milano->start_building_construction( &city_manager::civil_medium_appartment );
		city_milano->start_building_construction( &city_manager::civil_medium_appartment );

		city_milano->start_building_construction( &city_manager::civil_medium_house );
		city_milano->start_building_construction( &city_manager::civil_small_house );
		city_milano->start_building_construction( &city_manager::civil_small_appartment );
		city_milano->start_building_construction( &city_manager::civil_medium_appartment );
		city_milano->start_building_construction( &city_manager::civil_medium_appartment );
		city_milano->start_building_construction( &city_manager::civil_medium_appartment );

		city_milano->start_building_construction( &city_manager::civil_welfare_administration_office );*/

		city_roma->start_building_construction( &city_manager::civil_medium_house );
		city_roma->start_building_construction( &city_manager::civil_small_poor_commercial_building );
	//	city_roma->start_building_construction( &city_manager::civil_medium_house );
		city_roma->start_building_construction( &city_manager::civil_small_house );
	/*	city_roma->start_building_construction( &city_manager::civil_small_house );
		city_roma->start_building_construction( &city_manager::civil_medium_appartment );
		city_roma->start_building_construction( &city_manager::civil_medium_appartment );
		city_roma->start_building_construction( &city_manager::civil_medium_appartment );
		city_roma->start_building_construction( &city_manager::civil_medium_appartment );

		city_roma->start_building_construction( &city_manager::civil_medium_house );
		city_roma->start_building_construction( &city_manager::civil_medium_house );
		city_roma->start_building_construction( &city_manager::civil_small_house );
		city_roma->start_building_construction( &city_manager::civil_small_house );
		city_roma->start_building_construction( &city_manager::civil_medium_appartment );
		city_roma->start_building_construction( &city_manager::civil_medium_appartment );
		city_roma->start_building_construction( &city_manager::civil_medium_appartment );
		city_roma->start_building_construction( &city_manager::civil_medium_appartment );*/
	}

	game_map::gameplay_map* game_manager::get_the_game_map()
	{
		return &map;
	}
}

