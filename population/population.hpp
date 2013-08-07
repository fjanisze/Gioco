
#include "../logging/logger.hpp"
#include <cassert>
#include <map>

#define NOT_VALID_AGE -1
#define NOT_VALID_ID -1

namespace population
{
    //Gender type
    enum gender_t
    {
        male,
        female
    };

    //The assumption is that a standard family is composed by mather, father and eventually kids.
    enum family_status_t
    {
        is_single,
        is_mather,
        is_father,
        is_kid
    };

    //A population unit exist only in the context af a family_t. This structure contains very basic and 'personal'
    //information on a family member.
    struct population_unit_t
    {
        long unit_id;

        family_status_t status;
        gender_t gender;
        short age;

        population_unit_t( long id );
    };

    //The family is an atomic unit type, which may have different size and be composed by different type of members.
    //Is needed at least one person (unit) to have a family.
    class family_t
    {
        static long next_unit_id;
        long family_id; //Each family have its own, is unique!
        short amount_of_members; //Should be at least 1.
        population_unit_t* father; //When the family size is 0, only this field have a valid value
        population_unit_t* mather;
        long get_next_id();
    public:
        family_t( long id );
        ~family_t();
        short get_amount_of_members();
        long add_member( short age, family_status_t status, gender_t gender );
        long get_id();
    };

    typedef std::map< long , family_t* > family_map_t;
    typedef std::map< long , family_t* >::iterator family_container_iter;

    //A collection of families is a kind of aggregate which for example is used to track the families which live in a single building
    //All the families need to be part of a collection
    class family_collection_t
    {
        long collection_id;
        family_map_t collection;
    public:
        family_collection_t( long id );
        void add_family( family_t* family );
        long get_population();
    };

    typedef std::map< long , family_collection_t* >::iterator family_coll_iter;

    //Responsible for the population management.
    class population_manager
    {
        family_map_t family_container;//All the families are in this container
        static long next_family_id;
        static long next_collection_id;
        long get_next_family_id();
        long get_next_coll_id();
        //Collection data structures
        std::map< long , family_collection_t* > family_collections;
        long homeless_collection_id;
    public:
        family_t* create_family( );
        family_t* get_family( long id );
    public: //Collections
        long create_collection();
        family_collection_t* get_collection( long id );

    public: //Other
        population_manager();
        ~population_manager();
    };
}











