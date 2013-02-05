#include <string>
#include <ctime>
#include <queue>
#include <cmath>

namespace random
{
	class random_numbers
	{
		std::queue< long > rnd_db;
	public:
		random_numbers();
		long get_next_rnd_number();
		void regenerate_rnd_numbers();
	};
}
