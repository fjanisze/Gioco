#include "goods.hpp"

namespace goods_manager
{
	//Good descriptor definition
	const good_descriptor_t descriptor_food_bread = { 1 , "Bread", 2 , good_category_t::is_food , good_category_food_t::is_bread };
	const good_descriptor_t descriptor_food_meat = { 2 , "Meat", 11 , good_category_t::is_food , good_category_food_t::is_meat };
	const good_descriptor_t descriptor_food_fish = { 3 , "Fish", 27 , good_category_t::is_food , good_category_food_t::is_fish };


	const good_descriptor_t* food_descriptor_table[]
	{
		&descriptor_food_bread,
		&descriptor_food_meat,
		&descriptor_food_fish,
		nullptr
	};

	//////////////////////////////////////////////////////////////////
	//
	//
	//	 object definitions
	//
	//
	//////////////////////////////////////////////////////////////////
	
	good_food_obj::good_food_obj( const good_descriptor_t* desc ) 
	{
		descriptor = desc;
	}
	
	const good_descriptor_t* good_food_obj::get_descriptor()
	{
		return descriptor;
	}

	
	//////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for good_category_food_obj
	//
	//
	//////////////////////////////////////////////////////////////////


	//////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for goods_container and goods_manager
	//
	//
	//////////////////////////////////////////////////////////////////
	
	good_node::good_node()
	{
		good = nullptr;
	}

	good_node::good_node( good_obj* gd )
	{
		good = gd;
	}

	good_node::~good_node()
	{
		if( good )
		{
			delete good;
		}
	}

	goods_container::goods_container()
	{
		LOG("goods_container::goods_container(): Creating ");
		int amount_of_goods = 0;
		//Food category
		assert( create_new_category( good_category_t::is_food ) );
		good_category& cat = goods[ good_category_t::is_food ];
		//Add the goods 
		amount_of_goods += fill_from_table( cat.goods, food_descriptor_table );
	}

	int goods_container::fill_from_table( vector< good_node* > container , 
			const good_descriptor_t* table[] )
	{
		int amount = 0;
		for( int i = 0  ; table[ i ] != nullptr ; i++ )
		{
			container.push_back( create_node( table[ i ] ) );
			++amount;
		}
		return amount;
	}

	//Create the proper node on the base of the provided descriptor
	good_node* goods_container::create_node( const good_descriptor_t* desc )
	try{
		good_node* node = new good_node;
		switch( desc->category )
		{
			case good_category_t::is_food:
				node->good = new good_food_obj( desc ) ;
				break;
			default:
				LOG_ERR("goods_container::create_node(): Unknow category: ", desc->category );
				throw "Unknow categor!";
		}
		return node;
	}catch( std::exception& xa )
	{
		LOG_ERR("goods_container::create_node(): Exception: ", xa.what() );
		throw std::runtime_error("Unable to continue");
	}

	bool goods_container::create_new_category( good_category_t category )
	{
		bool verdict = false;
		if( goods.find( category ) == goods.end() ) //This category does not exist
		{
			ELOG("goods_container::create_new_category(): Adding: ", category );
			good_category new_category;
			new_category.category = category;
			goods[ category ] = new_category;
			verdict = true;
		}
		return verdict;
	}

	goods_manager::goods_manager()
	{
		LOG("goods_manager::goods_manager(): Created, registering the goods");
	}
}

