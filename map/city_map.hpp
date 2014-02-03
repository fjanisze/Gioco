
#ifndef CITYMAP_HPP
#define CITYMAP_HPP

#include "../logging/logger.hpp"
#include "map_common.hpp"
#include "../mytypes.hpp"
#include "../ui/draw.hpp"
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

    //Graphic information for a field
    struct field_graphic_info
    {
        //Graphic information for the field
        drawing_objects::drawable_object< sf::VertexArray > vertex;
        //Texture for this field
        sf::Texture* texture;

        field_graphic_info() : texture( nullptr )
        {      }
    };

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

        //Graphic information on this node
        field_graphic_info graphic_info;
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
        citymap::citymap_field_t* get_field( long field_id );
        field_graphic_info* get_field_graphic_info( long field_id );
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
	    citymap_container* map;
	    drawing_objects::drawing_facility* draw;
	    //ID of the context for the city
	    int city_graphic_context_id;
	    sf::VertexArray* create_single_vertex( node_t* node );
     protected:
	    //Information needed for the field creation
	    long field_width,
            field_height;
	    //Viewport information
	    game_map::game_canvas_settings_t game_canvas_setting;
	    void set_field_color( field_graphic_info* field_graphic, sf::Color new_color );
    public:
        citymap_container* set_citymap_container( citymap_container* city_map );
        void set_game_canvas_settings( game_map::game_canvas_settings_t canvas_setting );
        long create_vertex_map( const std::string& city_name );
        long get_field_width() { return field_width; }
        long get_field_height() { return field_height; }
        int get_graphic_context_id();
	};


    //handle the map for a specific city
	class citymap_t : public citymap_graphic_t
	{
	    citymap_field_container::citymap_container map;
        std::string city_name;
    private:
	    long next_field_id;
	    citymap_info_t city_info;
	    long get_next_id();
	    citymap_field_t* create_citymap_field( field_type_t type );
        bool fill_empty_map();
	public:
	    citymap_t( int map_size , game_map::game_canvas_settings_t game_canvas , const std::string& name );
	    citymap_field_t* get_field_at_pos( long x , long y );
	    citymap_field_t* get_field( long field_id );
	    ~citymap_t();
	    constructions::construction_t* set_construction( long field_id , constructions::construction_t* new_construction );
	};

};

#endif

















