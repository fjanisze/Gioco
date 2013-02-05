#include "random.hpp"

namespace random
{

	random_numbers::random_numbers()
	{
		rnd_db.reserve( 1000 );
	}

	inline
	long random_numbers::get_next_rnc_number()
	{
		long val = rnd_db.back();
		rnd_db.pop();
		return val;
	}

}
