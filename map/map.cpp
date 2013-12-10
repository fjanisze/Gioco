#define LOGGING_LEVEL_1
//#define LOGGING_LEVEL_2

#include "../logging/logger.hpp"
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
	    std::lock_guard< std::mutex > lock( map_mutex );
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
	    std::lock_guard< std::mutex > lock( map_mutex );
		const double tree_factor = 0.32;
		const short amount_of_crop = 200;
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

		for( long i = 0 , num_of_tree_inserted = 0 ; i < crop_coord.size() ; i++ )
		{
			current_field = crop_coord.back();
			crop_coord.pop_back();
			//Look for the nearest field, the idea is that the forest
			//will grow connecting one crop field to another
			nearest_field = first_closest_field( current_field, crop_coord );
			if( nearest_field >= 0 )
			{
				//We found something, now populate the fields between those two points
                num_of_tree_inserted = put_forest_between_two_fields( amount_of_trees_per_crop , current_field , crop_coord[ nearest_field ] );
				ELOG("gameplay_map::generate_random_map(): No more closest fields found, next_field.x < 0, amount of trees inserted ", num_of_tree_inserted , " max amount of trees for this crop ",amount_of_trees_per_crop );
				//Check if more trees need to be added
				if( num_of_tree_inserted < amount_of_trees_per_crop )
				{
					//More items have to be added
					ELOG("gameplay_map::generate_random_map(): Adding more trees near the crop.. " , amount_of_trees_per_crop - num_of_tree_inserted , " remaining" );
					while( num_of_tree_inserted < amount_of_trees_per_crop  )
					{
						tmp_coord = find_closest_field_of_type( current_field , &terrain_grass );
						if( are_coord_valid( tmp_coord ) )
						{
							add_obj_to_field( tmp_coord, &terrain_forest );
						}
						else break;
						++num_of_tree_inserted;
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

	//Populate the fields between two points
	long gameplay_map::put_forest_between_two_fields( long amount_of_trees_per_crop ,
            field_coordinate from , field_coordinate to )
	{
	    long amount_of_new_tree = 0;
	    field_coordinate current_field = from , next_field;
        for( long j = 0 ; j < amount_of_trees_per_crop ; j++)
        {
            if( !are_coordinates_equal( current_field, to ) )
            {
                next_field = path_find_next_field( current_field, to);
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
            ++amount_of_new_tree;
        }
        return amount_of_new_tree;
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
	field_manager* gameplay_map::create_a_city_at_random_coord( cities::city_agent* agent )
	{
		ELOG("gameplay_map::create_a_city_at_random_coord(): Creating \'", name  );
		field_manager* field = nullptr;
		//First of all, the proper object will be addeo
		field_coordinate city_coord = find_random_field( terrain_grass );
		field = create_a_city( agent , city_coord );
		return field;
	}

	//Create a city on a specific coordinates
    field_manager* gameplay_map::create_a_city( cities::city_agent* agent, const field_coordinate& city_coord )
    {
        ELOG("field_manager* create_a_city(): Creating the city");
        field_manager* field = nullptr;
        if( are_coord_valid( city_coord ) )
		{
			field = add_obj_to_field( city_coord , &terrain_city );
			//Add the city agent to the field
			field->add_city_agent( agent );
		}
		else
		{
			LOG_ERR("gameplay_map::create_a_city(): Unable to create the city!! Not valid coordinates");
		}
		return field;
    }

    ////////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for game_map
	//
	//
	////////////////////////////////////////////////////////////////////


    field_graphics_t::field_graphics_t( const map_field* m_field , long& cur_x, long& cur_y , long size_x , long size_y  )
    {
        assert( m_field != nullptr );
        //ELOG("field_graphics_t::field_graphics_t(): Field ID: ", m_field->field_id );
        field = m_field;
        manager = m_field->manager;
        descriptor = m_field->manager->get_visible_object();
        //Create and empty vertex
        sf::VertexArray* vrtx = new sf::VertexArray( sf::Quads , 4 );

        //Set the position
        (*vrtx)[0].position = sf::Vector2f( cur_x , cur_y );
        (*vrtx)[1].position = sf::Vector2f( cur_x + size_x, cur_y );
        (*vrtx)[2].position = sf::Vector2f( cur_x + size_x, cur_y + size_y );
        (*vrtx)[3].position = sf::Vector2f( cur_x , cur_y + size_y );

        vertex.update( vrtx );
    }

    field_graphics_t::~field_graphics_t()
    {
    }

    //Return true is the provided coordinate belong to this field
    bool field_graphics_t::is_within_the_field( long x, long y )
    {
        sf::VertexArray& vrtx = vertex.get();
        //Check the x coord
        if( x > vrtx[ 0 ].position.x && x <= vrtx[ 1 ].position.x )
        {
            //Check the y coord
            if( y > vrtx[ 0 ].position.y && y <= vrtx[ 2 ].position.y )
            {
                return true;
            }
        }
        return false;
    }

    game_map* game_map::instance = nullptr;

    game_map* game_map::get_instance()
    {
        if( instance == nullptr )
        {
            instance = new game_map;
        }
    }

    //Need to be called in order to configure properly the map
    bool game_map::configure_game_canvas( const game_canvas_settings_t& conf )
    try{
        LOG("game_map::configure_game_canvas(): Width: ",conf.canvas_width, ", Height: ",conf.canvas_height );
        //Make a local copy
        game_canvas = new game_canvas_settings_t;
        *game_canvas = conf;
        return true;
    }catch( std::exception& xa )
    {
        LOG_ERR("game_map::configure_game_canvas(): Exception catched, aborting ");
        return false;
    }

	//This function loop through all the map fields and create the relative vertex, return the amount of vertex
	long game_map::create_vertex_map()
	{
	    std::lock_guard< std::mutex > lock( map_mutex );
	    std::vector< map_field* >& map = get_fieldmap();
	    LOG("game_map::create_vertex_map(): Creating the vertex obj for the whole map. Map size: ", map.size() );
	    long amount_of_vertex = 0;
	    //Prepare the needed calculation constraint
	    long field_per_axis = std::sqrt( map.size() );
	    assert( field_per_axis > 0 );
	    long field_x_size = game_canvas->canvas_width / field_per_axis;
	    long field_y_size = game_canvas->canvas_height / field_per_axis;
	    long current_x_pos = game_canvas->canvas_x_offset,
            current_y_pos = game_canvas->canvas_y_offset;

        try{

            for( auto& elem : map )
            {
                field_graphics_t* field = new field_graphics_t( elem , current_x_pos , current_y_pos , field_x_size , field_y_size  );
                //update the current position
                current_x_pos += field_x_size;
                //Moving down of one fow?
                if( current_x_pos >= game_canvas->canvas_width )
                {
                    current_x_pos = 0;
                    current_y_pos += field_y_size;
                }
                set_vertex_texture( field );
                g_map.push_back( field );
                ++amount_of_vertex;
            }
            LOG("game_map::create_vertex_map(): Adding " , g_map.size() , " drawable objects to the drawing factory");
            //Adding all the drawable object to the drawing facility
            drawing_objects::drawing_facility* draw = drawing_objects::drawing_facility::get_instance();
            for( auto elem : g_map )
            {
                draw->add( &elem->vertex );
            }

        }catch( std::exception& xa )
        {
            LOG_ERR("game_map::create_vertex_map(): Exception: ", xa.what(), ", need to abort the execution!");
            amount_of_vertex = -1;
        }

	    return amount_of_vertex;
	}

	//Clear the used resources
    void game_map::destroy_vertex_map()
    {
        LOG("game_map::destroy_vertex_map(): Cleaning.." );
    }

	void game_map::set_vertex_texture( field_graphics_t* field )
	{
	    sf::VertexArray& vrtx = field->vertex.get();
	    //Different color/texture depending on the field type
	    map_common::object_descriptor* obj = field->field->manager->get_visible_object();

	    field->vertex.lock();
        for( short i = 0 ; i < 4 ; i ++  )
        {
            vrtx[ i ].color = obj->color;
        }
        field->vertex.unlock();
	}

	game_map::game_map()
	{
	    game_canvas = nullptr;
	}

	game_map::~game_map()
	{
	    if( game_canvas )
        {
            delete game_canvas;
        }
	}

	//Return the field object which belong to the provided coordinates
	field_graphics_t* game_map::get_field_at_pos( long x , long y )
	{
	    static field_graphics_t* field = nullptr;
	    if( field != nullptr )
        {
            if( field->is_within_the_field( x , y ) )
            {
                return field;
            }
            field = nullptr;
        }
        for( auto cur_field : g_map )
        {
            if( cur_field->is_within_the_field( x , y ) )
            {
                field = cur_field;
                break;
            }
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
	    city_agent = nullptr;
		//Default object is of a 'land' type
		object_descriptor* od =  create_new_obj_descriptor();
		*od = base_land;
		symbol = base_land.symbol;

		visible_obj_descriptor = od;

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
				visible_obj_descriptor = obj_d;
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

	//Return the descriptor of the 'visible' object
	object_descriptor* field_manager::get_visible_object()
	{
	    return visible_obj_descriptor;
	}

	obj_list_t& field_manager::get_obj_list()
	{
		return obj_list;
	}

	char field_manager::get_field_symbol()
	{
		return symbol;
	}

	void field_manager::add_city_agent( cities::city_agent* agent )
	{
	    city_agent = agent;
	}

	cities::city_agent* field_manager::get_city_agent()
	{
	    return city_agent;
	}
}

