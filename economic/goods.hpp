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

	//The leaf node will contain the data itself
	struct leaf_node_t
	{
	};

	//Node of the tree structure
	struct good_node_t
	{
		//Parental hierarchies
		good_node_t* child_nodes,
				* parent_node;
		//Link to the nodes at the same level
		good_node_t* next,
			*prev;
		//Eventually, link to the leaf node
		leaf_node_t* leaf_node; 

		good_node_t():
	};

	//Good container
	class good_container
	{
		good_node_t* root; //of the whole structure structure
		good_node_t* create_node();
	public:
		good_container();
	};
}



#endif

