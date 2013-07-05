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
        for( long node_i = 0 ; node_i < row_size ; node_i++ )
        {
            set_topbottom_link( all_nodes[ node_i ] , row_size );
        }
    }

    void citymap_container::set_topbottom_link( node_t* node , long size )
    {
        if( node->adjacent_node[ ad_node::bottom ] == nullptr )
        {
            //Are we in the last row?
            if( node->node_index >= ( amount_of_nodes - size ) )
            {
                //Link with the first row
                long row_index = node->node_index % size;
                node->adjacent_node[ ad_node::bottom ] = all_nodes[ row_index ];
                //Link with the last row..
                all_nodes[ row_index ]->adjacent_node[ ad_node::top ] = node;
            }
            else
            {
                node->adjacent_node[ ad_node::bottom ] = all_nodes[ node->node_index + size ];
                set_topbottom_link( node->adjacent_node[ ad_node::bottom ] , size );
            }
        }
    }
}

namespace citymap
{

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

    citymap_field_t::citymap_field_t() : field_id( 0 )
    {
        vertex = sf::VertexArray( sf::Quads , 4 );
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
        assert( map_size > 0 );
        city_info.set_info( map_size );
    }

    long citymap_t::get_next_id()
    {
        return next_field_id++;
    }
}
