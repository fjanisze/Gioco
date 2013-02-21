#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "economic.hpp"
#include "../logging/logger.hpp"
#include "../common_structures.hpp"
#include <string>
#include <map>
#include <iostream>

#ifndef GOODS_HPP
#define GOODS_HPP

namespace goods_manager
{
	using namespace finance;
	using namespace std;
	//General goods classification
	typedef enum good_category_t 
	{
		is_food = 0,
		is_beverage,
	} good_type_t;

	typedef long sub_category_index_t;

	//Each category have a some possible subcategories
	//For the food
	typedef enum good_category_food_t
	{
		is_bread = 0,
		is_meat,
		is_fish,
		is_tomato,
		is_pasta,
		is_rice,
		is_vegetables,
		is_fruit
	} good_category_food_t;
	//For the beverage
	typedef enum good_catetory_beverage_t
	{
		is_water = 0,
		is_juice,
		is_wine,
		is_alcohol,
		is_milk
	} good_catetory_beverage_t;

	//Descriptor with some basic information
	typedef struct good_descriptor_t
	{
		long obj_id;
		string name;
		currency_type base_price;
		good_category_t category;
		sub_category_index_t sub_category_index;
	} good_descriptor_t;

	//Possible descriptors
	extern const good_descriptor_t descriptor_food_bread;
	extern const good_descriptor_t descriptor_food_meat;
	extern const good_descriptor_t descriptor_food_fish;

	extern const good_descriptor_t* food_descriptor_table[];

	class good_obj
	{
	protected:
		const good_descriptor_t* descriptor;
	public:
		virtual const good_descriptor_t* get_descriptor() = 0;
	};

	class good_food_obj : public good_obj 
	{
	public:
		good_food_obj( const good_descriptor_t* desc );
		const good_descriptor_t* get_descriptor();
	};

	//////////////////////////////////////////////////////////////////
	//
	//
	//	good_manager
	//
	//
	//////////////////////////////////////////////////////////////////

	typedef int category_index; 

	struct good_node
	{
		good_obj* good;

		good_node();
		good_node( good_obj* gd );
		~good_node();
	};

	struct good_category
	{
		good_category_t category;
		string name,
		       description;
	
		//List of goods
		vector< good_node* > goods;
	};

	class goods_container
	{
		map< category_index , good_category > goods;
		bool create_new_category( good_category_t category );
		good_node* create_node( const good_descriptor_t* desc );
		int fill_from_table( vector< good_node* > container , const good_descriptor_t* table[] );
	public:
		goods_container();
	};

	class goods_manager : public goods_container
	{
	public:
		goods_manager();
	};
}



#endif

