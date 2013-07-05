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
        vertex = sf::VertexArray( sf::Quads , 4 );
    }

    city_field_descriptor* citymap_field_t::new_descriptor() throw( std::bad_alloc )
    {
        return new city_field_descriptor;
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

    citymap_t::citymap_t( int map_size ) : next_field_id( 1 )
    {
        LOG("citymap_t::citymap_t(): Creating a new citymap container ");
        assert( map_size > 0 );
        city_info.set_info( map_size );
        //Create the container for the fields
        map.create( map_size );
        //Fill with grass fields
        assert( fill_empty_map() );

    }

    citymap_t::~citymap_t()
    {
        map.clear_all_the_field();
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
         //The only admitted terrain at the moment is a 'grass', fill with grass everywhere
         for( long index = 0 ; index < map.get_size() ; index++ )
         {
             citymap_field_t* field = create_citymap_field( field_type_t::grass_field );
             if( field )
             {
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
}
