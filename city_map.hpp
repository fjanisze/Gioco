
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
#include <mutex>

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

        //Graphic information for the field
        std::vector< sf::VertexArray* > vertex;
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
        ~citymap_container();
        node_t* create( long size );
        node_t* set_field( long index , citymap::citymap_field_t* field );
        long clear_all_the_field();
        long get_size() { return amount_of_nodes; }
        bool empty() { return amount_of_nodes == 0; }
        std::vector< node_t* >::iterator begin();
        std::vector< node_t* >::iterator end();
        typedef std::vector< node_t* >::iterator iterator;
        citymap::citymap_field_t* get_field( long row, long column );
    };
}

namespace citymap
{
	using namespace map_common;

	//Admitted type for the field
	extern const city_field_descriptor city_field_grass;


    using namespace citymap_field_container;

	//This class is responsible for the handling of the graphical appearence of the city.
	class citymap_graphic_t
	{
        std::mutex mutex;
	    //Copy of the game map
	    citymap_container* map;
	    //Vertex information
	    std::vector< sf::VertexArray* > vertex;
	    sf::VertexArray* create_single_vertex( node_t* node );
     protected:
	    //Information needed for the field creation
	    long field_width,
            field_height;
	    //Viewport information
	    game_map::map_viewport_settings_t viewport_settings;
    public:
        citymap_container* set_citymap_container( citymap_container* city_map );
        void set_viewport_settings( game_map::map_viewport_settings_t viewport );
        long create_vertex_map();
        void clear_all_vertex();
        std::vector< sf::VertexArray* >* get_city_vertex();
	};


    //handle a city object
	class citymap_t : public citymap_graphic_t
	{
	    citymap_field_container::citymap_container map;
    private:
	    long next_field_id;
	    citymap_info_t city_info;
	    long get_next_id();
	    citymap_field_t* create_citymap_field( field_type_t type );
        bool fill_empty_map();
	public:
	    citymap_t( int map_size , game_map::map_viewport_settings_t viewport  );
	    citymap_field_t* get_field_at_pos( long x , long y );
	    ~citymap_t();
	};


	class city_agent;

	//Handle the cities
	class city_manager
	{
	    static long next_city_id;
	    std::vector< city_info_t* > city_container; //All the cities are contained in this vector
	    //Each city have its own city_agent
	    std::vector< city_agent* > agents;
	    //The viewport settings are needed for the city map creation
	    game_map::map_viewport_settings_t viewport_settings;
	    long get_next_city_id();
	    city_info_t* find_city_info( long city_id );
	    city_agent* create_new_agent( city_info_t* city_info );
    public:
        city_agent* get_city_agent( long city_id );
        long get_city_id( const std::string& name );
        city_manager( game_map::map_viewport_settings_t viewport );
        city_agent* create_new_city( const std::string& name , long size );
	};
}

#endif

















