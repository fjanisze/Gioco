#ifndef COMMON_STRUCTURES_HPP
#define COMMON_STRUCTURES_HPP

namespace player_info
{
	struct player_manager;
}

namespace city_manager
{
	class city_manager;
}

namespace population
{
	class population_manager;
}

namespace economics
{
	class economy_manager;
}

namespace game_manager
{
	class game_manager;
	struct player_game_objects
	{
		player_info::player_manager*    player;
		city_manager::city_manager*     cities;
		population::population_manager* population;
		economics::economy_manager*     economics;
	};
}

#endif
