#define LOGGING_LEVEL_1

#include "logging/logger.hpp"
#include "map.hpp"

namespace game_objects
{

	bool is_a_terrain_object(const object_descriptor& obj)
	{
		if( obj.obj_id < 10 ) 
			return true;
		return false;
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
		ELOG("gameplay_map::add_obj_to_field(): Parm: ", coord.x, ":", coord.y," -> ", obj->name.c_str());
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
					ELOG("gameplay_map::generate_random_map(): Adding more trees near the crop.. " , amount_of_trees_per_crop - j , " remaining" );
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

