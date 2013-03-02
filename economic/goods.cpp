#include "goods.hpp"

namespace goods_manager
{
	good_node_t::good_node_t() : child_node( nullptr ), parent_node( nullptr ), leaf_node( nullptr ),
		next( nullptr ), prev( nullptr )
	{
	}

	//Create and empty node
	good_node_t* good_container::create_node()
	try{
		good_node_t* node = new good_node_t;
		return node;
	}catch( std::exception& xa )
	{
		LOG_ERR("good_container::create_node(): Exception catched, what: ",xa.what() );
		return nullptr;
	}

	//Contructor
	good_container::good_container() : root( nullptr )
	{
		//Create an empty container;
		root = create_node();
	}
}

