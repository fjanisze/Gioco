
#ifndef CITYMAP_HPP
#define CITYMAP_HPP

#include "logging/logger.hpp"
#include "map_common.hpp"
#include "mytypes.hpp"
#include <string>
#include <cassert>
#include <SFML/Graphics.hpp>
#include <vector>
#include <algorithm>

namespace citymap
{
    struct citymap_field_t;
}

namespace citymap_field_container
{
    /* Structure which contain the fields for the city */

    //Adjancent nodes index
    typedef enum ad_node
    {
        right,
        bottom,
        left,
        top
    } ad_node;

    //Each node is connected with at least four other adiacent nodes
    struct node_t
    {
        //Unique index for the node
        long node_index;
        /*
            Adjacent nodes:
             [ 0 ] The node on the rigth. The last node of the row is connected with the first of the next row
             [ 1 ] The node below this one, the last one of the column is connected with the first of the next column
             [ 2 ] The node on the left, the last one is connected with the fist of the previous row.
             [ 3 ] The node over, same rule as for the other nodes.
        */
        node_t* adjacent_node[ 4 ];

        node_t();

        //The field contained in this node
        citymap::citymap_field_t* field;
    };

    //The container
    class citymap_container
    {
        long amount_of_nodes;
        std::vector< node_t* > all_nodes; //Contains the pointers for all the node
        node_t* first_node; //Upper left corner
        void set_all_the_links( node_t* node , long size );
        void set_topbottom_link( node_t* node , long size );
        bool check_map_size( long size );
    public:
        citymap_container();
        node_t* create( long size );
    };
}

namespace citymap
{
	using namespace map_common;

	struct city_field_descriptor
	{
		mlong obj_id;

		std::string name,
                desc;
	};

	//Contains some information on the city
	struct citymap_info_t
	{
	    int size;
	    long amount_of_fields; //Which is just size * size

	    //Utility constructors ecc
	    citymap_info_t();
	    void set_info( int map_size );
	};

	//City field
	struct citymap_field_t
	{
	    //ID for the field
	    mlong field_id;
	    //Coordinate of the field
	    field_coordinate coord;

	    //Related vertex
	    sf::VertexArray vertex;

	    citymap_field_t();
	};


    //handle a city object
	class citymap_t
	{
	    citymap_field_container::citymap_container map;
    private:
	    long next_field_id;
	    citymap_info_t city_info;
	    long get_next_id();
	public:
	    citymap_t( int map_size );
	};
}

#endif

















