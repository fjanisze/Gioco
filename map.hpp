#ifndef MAP_HPP
#define MAP_HPP

#include "mytypes.hpp"
#include "map_common.hpp"
#include <map>
#include <vector>
#include <string>
#include <list>
#include <algorithm>
#include <SFML/Graphics.hpp>
#include <cassert>

using namespace std;
using namespace map_common;

namespace game_objects
{

	typedef enum object_type
	{
		obj_land = 0,
		obj_building
	}object_type;

	//Some standard objects
	static const object_descriptor base_land = { 1, "Land" , "Piece of land, nothing more", 100, 'X' };
	static const object_descriptor terrain_grass = { 2, "Grass", "Fresh grass", 200, '.'};
	static const object_descriptor terrain_forest = { 3, "Forest", "A lot of tree", 600 , 'F' };
	static const object_descriptor terrain_dummy = { 4, "DUMMY", "A dummy terrain", 0 , '?' };
	static const object_descriptor terrain_city = { 5, "City", "In this field a city was deployed", 0 , 'C' }; //The value of those fields depend on the items present in the city
	//Some specific objects

	bool is_a_terrain_object(const object_descriptor& obj);
}

namespace game_map
{

	using namespace game_objects;

	static const mlong base_field_value = 1000;
	static const char not_discovered_field_symbol = '?';

	//declaration for field_manager
	class field_manager;

	typedef list< object_descriptor* > obj_list_t;

	//map_field collect many information about a field
	struct map_field
	{
		long field_id;
		field_coordinate coord;
		short state;
		int owner;
		long value;
		field_manager* manager;

		~map_field();
		map_field();
	};

	//Class with calculation support functions
	class map_calculation
	{
		typedef field_coordinate fc;
	public:
		double distance(const fc& point1, const fc& point2);
		long first_closest_field( const fc& origin, const vector< fc >& other_fields );
		fc path_find_next_field( const fc& origin, const fc& dest );
		bool are_coordinates_equal( const fc& point1, const fc& point2 );
		list< field_coordinate > get_field_perimeter( const field_coordinate& origin, int range );
	};

	typedef vector< field_coordinate > vec_field_coord;

	class gameplay_map : public map_calculation
	{
		vector< map_field* > map;
		long map_size;
		long num_of_fields;
		void set_invalid_coord( field_coordinate& coord );
		bool check_field_type_presence( const field_coordinate& coord,  const object_descriptor& expected_obj );
	public:
		field_coordinate find_closest_field_of_type( const field_coordinate& origin, const object_descriptor* type );
	public:
		void create_new_map( long size );
		gameplay_map();
		gameplay_map(long size);
		~gameplay_map();
		field_manager* add_obj_to_field(const field_coordinate& coord, const object_descriptor* obj);
		void generate_random_map();
		vector< map_field* >& get_fieldmap() { return map; }
		long get_map_size() { return map_size; };
		long calculate_index( const field_coordinate& coord);
		bool are_coord_valid( const field_coordinate& coord);
		field_coordinate find_random_field( const object_descriptor& type );
	public:
		void make_all_map_explored();
		field_manager* create_a_city_at_random_coord( const string& name );
	};

	//This is the structure which needs to be used to configure the graphic apparence for the map
	//On the base of those values the proper vertex are created.
	struct map_viewport_settings_t
	{
	    //Those values are in pixels
	    long map_width,
            map_height;
	};

	//Object reppresenting the graphical reppresentation of the field
	struct field_graphics_t
	{
        const map_field* field; //Relative field
	    sf::VertexArray* vertex; //For the graphical rappresentation

	    field_graphics_t( const map_field* m_field );
	    ~field_graphics_t();
	};

	//This is the higher abstraction level for the game map, it handle vertex and so on
	class game_map : public gameplay_map
	{
	    map_viewport_settings_t* settings;
	    std::vector< field_graphics_t > g_map; //g_ stand for graphic
    public:
        game_map();
        ~game_map();
        bool configure_viewport( const map_viewport_settings_t& conf );
        void set_proper_vertex_position( sf::VertexArray* vertex , long& cur_x, long& cur_y , long size_x , long size_y );
        long create_vertex_map();
	};

	//field_manager is responsible to managing the objects that lead on a field

	class field_manager
	{
		obj_list_t obj_list; //List of objects present on this field
		char symbol; // graphical symbol for this field
		object_descriptor* create_new_obj_descriptor();
	public:
		field_manager();
		bool add_object(const object_descriptor* obj);
		~field_manager();
	public:
		obj_list_t& get_obj_list();
		char get_field_symbol();
	};
}

#endif
