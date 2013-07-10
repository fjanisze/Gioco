#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "city_map.hpp"

namespace citymap_field_container
{
    ////////////////////////////////////////////////////////////////////
    //
    //
    //	Follow the implementation for citymap_field_t and citymap_container
    //
    //
    ////////////////////////////////////////////////////////////////////

    citymap_container::citymap_container() : first_node( nullptr ) , amount_of_nodes( -1 )
    {
    }

    citymap_container::~citymap_container()
    {
        //Clear the node, if any
        for( auto node : all_nodes )
        {
            delete node;
        }
    }

    node_t::node_t() : node_index( 0 ) , field( nullptr )
    {
        for( short i = 0 ; i < 4 ; i++ )
        {
            adjacent_node[ i ] = nullptr;
        }
    }

    //Return true if the map size if acceptable
    bool citymap_container::check_map_size( long size )
    {
        double sq = sqrt( size );
        long iv = ( long ) sq;
        sq -= ( double ) iv;
        return sq == 0;
    }

    //Create the map, return the pointer for the first node. Size is the total amount of fields
    node_t* citymap_container::create( long size )
    try{
        LOG("citymap_container::create(): Creating a new container, size: ", size );
        //Check if the size is acceptable, allowing the map to be a NxN matrix

        if( !check_map_size( size ) )
        {
            LOG_ERR("citymap_container::create(): Map size not valid.. ");
            return nullptr;
        }
        node_t* node = nullptr;
        node = new node_t[ size ];
        amount_of_nodes = size;
        //Set all the indexes
        for( long node_i = 0 ; node_i < size ; node_i++ )
        {
            node[ node_i ].node_index = node_i;
        }
        //Set the links
        set_all_the_links( node , size );

        return node;
    }catch( std::exception& xa )
    {
        LOG_ERR("citymap_container::create(): Exception caught, ",xa.what() , ", Aborting" );
        std::exit( -1 );
    }

    void citymap_container::set_all_the_links( node_t* node , long size)
    {
        LOG("citymap_container::set_all_the_links(): Creating all the links in the container ");
        node_t *prev_node = nullptr , *cur_node = nullptr;

        first_node = node;
        cur_node = node;
        //Add the first node
        all_nodes.push_back( cur_node );
        //Inser the other nodes in the vector, and set the right/left link
        for( long node_i = 1 ; node_i < size ; node_i++ )
        {
            //Set the proper pointer
            prev_node = cur_node;
            cur_node = &node[ node_i ];
            //Set the left/right links
            cur_node->adjacent_node[ ad_node::left ] = prev_node;
            prev_node->adjacent_node[ ad_node::right ] = cur_node;
            //Push into the structure
            all_nodes.push_back( cur_node );
        }
        //Here, node point to the first node, and cur_node to the last, close the circle setting the laft/right poiters
        node->adjacent_node[ ad_node::left ] = cur_node;
        cur_node->adjacent_node[ ad_node::right ] = node;
        //Now set the bottom/top pointers
        long row_size = std::sqrt( size );
        set_topbottom_link( first_node , row_size );
    }

    //The right/left link should be already be setup before this function is called.
    void citymap_container::set_topbottom_link( node_t* node , long size )
    {
        node_t* last_node = nullptr;
        if( node->adjacent_node[ ad_node::bottom ] == nullptr )
        {
            //Check if we are in the last row
            if( node->node_index >= ( amount_of_nodes - size ) )
            {
                //Check if this is the last node in the whole structure (right down corner)
                if( node->node_index == ( amount_of_nodes - 1 ) )
                {
                    node->adjacent_node[ ad_node::bottom ] = first_node;
                    first_node->adjacent_node[ ad_node::top ] = node;
                    //Close the recursion
                    return;
                }
                {
                    last_node = all_nodes[ node->node_index % size ];
                    last_node = last_node->adjacent_node[ ad_node::right ];
                }
            }
            else
            {
                last_node = all_nodes[ node->node_index + size ];
            }
            node->adjacent_node[ ad_node::bottom ] = last_node;
            last_node->adjacent_node[ ad_node::top ] = node;
            set_topbottom_link( node->adjacent_node[ ad_node::bottom ] , size );
        }
    }

    std::vector< node_t* >::iterator citymap_container::begin()
    {
        return all_nodes.begin();
    }

    std::vector< node_t* >::iterator citymap_container::end()
    {
        return all_nodes.end();
    }

    //Set a field
    node_t* citymap_container::set_field( long index , citymap::citymap_field_t* field )
    {
        all_nodes[ index ]->field = field;
        return all_nodes[ index ];
    }

    long citymap_container::clear_all_the_field()
    {
        LOG("citymap_container::clear_all_the_field(): Removing all the field obj from the nodes");
        long count = 0;
        for( auto node: all_nodes )
        {
            if( node->field )
            {
                delete node->field;
                ++count;
            }
        }
        return count;
    }
}

namespace citymap
{

    //Default possible types for the field.
	const city_field_descriptor city_field_grass = { 0 , "Grass", "Fresh green grass" , field_type_t::grass_field };


    citymap_info_t::citymap_info_t()
    {
        size = 0;
        amount_of_fields = 0;
    }

    void citymap_info_t::set_info( int map_size )
    {
        size = map_size;
        amount_of_fields = map_size * map_size;
    }

    citymap_field_t::citymap_field_t() : field_id( 0 ) , descriptor( nullptr )
    {
    }

    city_field_descriptor* citymap_field_t::new_descriptor() throw( std::bad_alloc )
    {
        descriptor = new city_field_descriptor;
        return descriptor;
    }

    citymap_field_t::~citymap_field_t()
    {
        if( descriptor )
        {
            delete descriptor;
        }
    }

    ////////////////////////////////////////////////////////////////////
    //
    //
    //	Follow the implementation for citymap_t
    //
    //
    ////////////////////////////////////////////////////////////////////

    citymap_t::citymap_t( int map_size , game_map::map_viewport_settings_t viewport ) : next_field_id( 1 )
    {
        LOG("citymap_t::citymap_t(): Creating a new citymap container ");
        assert( map_size > 0 );
        city_info.set_info( map_size );
        //Create the container for the fields
        if( map.create( map_size ) == nullptr )
        {
            LOG_ERR("citymap_t::citymap_t(): Map size creation failed, aborting the initialization..");
        }
        else
        {
            //Fill with grass fields
            assert( fill_empty_map() );
            //Provide the pointer to the graphic manager class
            set_citymap_container( &map  );
            set_viewport_settings( viewport );
            //Create the map
            create_vertex_map();
        }
    }

    citymap_t::~citymap_t()
    {
        map.clear_all_the_field();
        clear_all_vertex();
    }

    long citymap_t::get_next_id()
    {
        return next_field_id++;
    }

    //Return a new empty citymap_field
    citymap_field_t* citymap_t::create_citymap_field( field_type_t type )
    try{
        citymap_field_t* field = new citymap_field_t;
        //If is not a custom type, set the proper descriptor. Otherwise the desciptor is absent
        if( type != field_type_t::custom_field )
        {
            switch( type )
            {
            case field_type_t::grass_field:
                *( field->new_descriptor() ) = city_field_grass;
                break;
            default:
                LOG_ERR("citymap_t::create_citymap_field(): Unknow field type: ", type );
                delete field;
                field = nullptr;
            };
        }
        //Set the ID
        field->field_id = get_next_id();
        return field;
    }catch( std::bad_alloc& xa )
    {
        LOG_ERR("citymap_t::create_citymap_field(): Allocation failure!");
        return nullptr;
    }

    //This function just fill the city map with terrains and no buildings
    bool citymap_t::fill_empty_map()
    {
         LOG("citymap_t::fill_empty_map(): Fill the map with grass ");
         long city_edge_size = std::sqrt( map.get_size() );
         field_coordinate coord;
         //The only admitted terrain at the moment is a 'grass', fill with grass everywhere
         for( long index = 0 ; index < map.get_size() ; index++ )
         {
             citymap_field_t* field = create_citymap_field( field_type_t::grass_field );
             if( field )
             {
                 //Fill the coordinate
                 if( index > 0 )
                 {
                     if( (index % city_edge_size ) == 0 )
                     {
                         ++coord.y;
                         coord.x = 0;
                     }
                     else
                     {
                         ++coord.x;
                     }
                 }
                 field->coord = coord;
                 //Add the field
                 map.set_field( index , field );
             }
             else
             {
                 LOG_ERR("citymap_t::fill_empty_map(): Not valid field, aborting" );
                 return false;
             }
         }
         return true;
     }

    ////////////////////////////////////////////////////////////////////
    //
    //
    //	Follow the implementation for citymap_graphic_t
    //
    //
    ////////////////////////////////////////////////////////////////////

    void citymap_graphic_t::set_viewport_settings( game_map::map_viewport_settings_t viewport )
    {
        viewport_settings = viewport;
    }

    citymap_container* citymap_graphic_t::set_citymap_container( citymap_container* city_map )
    {
        LOG("citymap_graphic_t::set_citymap_container(): ADDR ", &map , " map size: ",city_map->get_size() );
        map = city_map;
    }

     //Create the vertex map for all the city
     long citymap_graphic_t::create_vertex_map( )
     {
         LOG("citymap_graphic_t::create_vertex_map(): Size of the map: ", map->get_size() );
         std::lock_guard< std::mutex > lock( mutex );
         long amount_of_vertex = 0;
         if( !map->empty() )
         {
             //Set the field sizes
             field_width = viewport_settings.map_width / std::sqrt( map->get_size() ),
             field_height = viewport_settings.map_height / std::sqrt( map->get_size() );
             LOG("citymap_graphic_t::create_vertex_map(): Field size in pixel: ",field_width, ",", field_height );
             //To create the map the function need to collect from each field the information on the object present on them.
             //Actually the mapping is pretty simple, depending on the object type in the description, a different color is picked.
             citymap_container::iterator map_iter = map->begin() , map_end = map->end();
             while( map_iter != map_end )
             {
                 node_t* node = *map_iter;
                 //Create the vertex
                 sf::VertexArray* node_vertex = create_single_vertex( node );
                 if( !node_vertex )
                 {
                     //Quit
                     LOG_ERR("citymap_graphic_t::create_vertex_map(): Not all the vertex are ready to be used, only ",amount_of_vertex," created.. ");
                     break;
                 }
                 //Add the vertex
                 vertex.push_back( node_vertex );
                 ++map_iter;
                 ++amount_of_vertex;
             }
         }
         return amount_of_vertex;
     }

     //Create a single vertex for the provided node
     sf::VertexArray* citymap_graphic_t::create_single_vertex( node_t* node )
     {
         assert( node != nullptr );
         sf::VertexArray* ver = new(std::nothrow) sf::VertexArray( sf::Quads, 4 );
         if( ver == nullptr )
         {
             LOG_ERR("citymap_graphic_t::create_single_vertex(): Not able to allocate the memory for the vertex object..");
             return nullptr;
         }
         //Go ahead with the vertex creation.
         field_coordinate coord = node->field->coord;
         long field_off_x = field_width * coord.x ,
            field_off_y = field_height * coord.y;
         //Fill the position info
         (*ver)[0].position = sf::Vector2f( field_off_x , field_off_y );
         (*ver)[1].position = sf::Vector2f( field_off_x + field_width, field_off_y );
         (*ver)[2].position = sf::Vector2f( field_off_x + field_width , field_off_y + field_height  );
         (*ver)[3].position = sf::Vector2f( field_off_x , field_off_y + field_height );
         //Pick the color
         sf::Color field_color = sf::Color::Black;
         switch( node->field->descriptor->field_type )
         {
         case field_type_t::grass_field:
            //Set it red
            field_color = sf::Color::Green;
            break;
         default:
            break;
         };
         ELOG("citymap_graphic_t::create_single_vertex(): New vertex ready, offset x: ",field_off_x,", offset y:", field_off_y, ", height: ",field_height , ", width: ",field_width );
         return ver;
     }

     //Clear the vertex deleting them
     void citymap_graphic_t::clear_all_vertex()
     {
         std::lock_guard< std::mutex > lock( mutex );
         LOG("citymap_graphic_t::clear_all_vertex(): Amount of vertex ", vertex.size() );
         for( auto elem : vertex )
         {
             delete elem;
         }
     }
}































