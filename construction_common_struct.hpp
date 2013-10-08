#ifndef CONSTRUCTION_COMMON_STRUCT_HPP
#define CONSTRUCTION_COMMON_STRUCT_HPP

#include "mytypes.hpp"
#include <string>

namespace constructions
{
    typedef enum construction_type_t
	{
		appartment,
		office,
		error
	} construction_type_t;

	extern const std::string building_types_str[];

	//Include some common information about the construction, used during the construction parsing.
	struct construction_info
	{
		long obj_id;

		//Generic info
		construction_type_t type;

		std::string name,
			description;

		//Basic information about the construction time
		mlong construction_time; //in game cycles

		mlong price;

		construction_info() = default;
	};

    //This is a construction object, which should be present on a field
    class construction_t
    {
    protected:
        long obj_id;
        //Type of construction
        construction_type_t type;
        //Generic information, from the relative descriptor
        std::string name,
                    description;
        //Unit information
        long unit_size,
            unit_capacity; //The total capacity of the building is unit_size * unit_capacity

        //The price in the descriptor may be not valid anymore, a perks can influence the price
        mlong unit_price;
    public:
        construction_type_t get_construction_type();
        long get_obj_id();
        std::string get_name();
        friend class construction_manager;
    };

    //This construction is still ongoing
    class construction_ongoing : public construction_t
    {
    public:
        construction_ongoing();
    };

    //Appartment
    class construction_appartment : public construction_t
    {
    public:
        construction_appartment();
    };

    //Office
    class construction_office : public construction_t
    {
    public:
            construction_office();
    };
}

#endif


















