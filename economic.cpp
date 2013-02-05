#include "economic.hpp"

namespace finance
{
	game_wallet& game_wallet::operator=( const currency_type& value )
	{
		money_cash = value;
		return *this;
	}

	game_wallet::game_wallet( const currency_type& value )
	{
		money_cash = value;
	}

	game_wallet::game_wallet( const game_wallet& wallet )
	{
		money_cash = wallet.money_cash;
	}

	currency_type game_wallet::available_free_cash()
	{
		return money_cash;
	}

}

namespace economics
{

	//The sum of all the budgets detraction in percentage shall not be greather than 100!
	public_welfare::public_welfare()
	{
		poor_subsidies_budget = 5.0; // default value.
		poor_subsidies_fund = 0;
		poor_subsidies_request_counter = 0;
		poor_subsidies_total_funding = 0;
		is_available = false;
		LOG("public_welfare::public_welfare(): poor_subsidies_budget: ",poor_subsidies_budget,"%" );
	}

	bool public_welfare::is_welfare_available()
	{
		return is_available;
	}

	void public_welfare::set_welfare_availability( bool status )
	{
		is_available = status;
	}

	//The function will distribute the revenue between the funds and return the remaining value
	currency_type public_welfare::walfare_distribute_revenue( currency_type revenue )
	{
		currency_type tmp_value = 0, remaining_cash = revenue;
		if( is_welfare_available() && revenue >= 0 )
		{
			//Poor subsidies fund
			tmp_value = revenue * poor_subsidies_budget / 100;
			poor_subsidies_fund += tmp_value;
			remaining_cash -= tmp_value;	

			ELOG("public_welfare::walfare_distribute_revenue(): Poor fund: ", poor_subsidies_fund );
		}
		return remaining_cash;
	}

	void public_welfare::zeroes_poor_subsidies_counter()
	{
		poor_subsidies_request_counter = 0;
	}

	long public_welfare::get_poor_subsidies_counter()
	{
		return poor_subsidies_request_counter;
	}

	currency_type public_welfare::get_poor_subsidies_funds()
	{
		return poor_subsidies_fund;
	}

	//Check if this eu can access the welfare program
	bool public_welfare::can_access_poor_welfare_program( currency_type needed_money )
	{
		if( !is_welfare_available() )
		{
			return false; //The welfare is not available at all.
		}
		ELOG("public_welfare::can_access_poor_welfare_program(): Requested: ",needed_money,", available money: ",poor_subsidies_fund);
		//Check if the request does not exceed the 
		return needed_money <= poor_subsidies_fund;
	}

	bool public_welfare::is_poor_welfare_program_active()
	{
		return poor_subsidies_fund > 0;
	}

	//Provide the money from the poor subsidies fund
	currency_type public_welfare::access_poor_subsidies_fund( currency_type needed_money )
	{
		poor_subsidies_fund -= needed_money;
		++poor_subsidies_request_counter;
		poor_subsidies_total_funding += needed_money;
		return needed_money;
	}

	currency_type public_welfare::get_poor_subsidies_total_funding()
	{
		return poor_subsidies_total_funding;
	}

	//Set the default values for the economy variables
	economic_variables::economic_variables()
	{
		LOG("economic_variables::economic_variables(): Setting the default economic variables");
		/*
		 * Init the revenue_class table, those are gross yearly salaries.
		 * The difference about one class and another shall be at least SALARY_CLASS_MIN_DELTA
		 */
		salary_class[0] = 25000; //Very poor!
		salary_class[1] = 60000; //Still poor
		salary_class[2] = 144000; //Middle class, can effort for many high-value goods
		salary_class[3] = 300000; //Lower bound high-class, the average gross salary is quite high
		salary_class[4] = 900000; //Higher boud high-class
		salary_class[5] = 2520000; //The highest level.

		//Taxes
		salary_taxes[0] = 19.5; //Lower lowel, for the poors.
		salary_taxes[1] = 25;
		salary_taxes[2] = 29.5;
		salary_taxes[3] = 37.0;
		salary_taxes[4] = 47.0;
		salary_taxes[5] = 57.0; //For the most wealthy people

		//Default salary assigned to each possible class, yearly value.
		default_class_salary[0] = 19000;
		default_class_salary[1] = 45000;
		default_class_salary[2] = 120000;
		default_class_salary[3] = 250000;
		default_class_salary[4] = 600000;
		default_class_salary[5] = 1500000;

		//Set other variables
		amount_of_starving_unit = 0;
		total_eu_population = 0;
		salary_taxes_equity_perception = 0.5; //This value is changed at every review_economy loop, when the net salary is distributed
	}

	long economic_unit::eu_next_unique_id = 1;

	economic_unit::economic_unit()
	{
		eu_id = eu_next_unique_id++;
		//At the very beginning the eu has nothing.
		gross_salary = 0;
		wallet = 0;
		family_size = nullptr;
		starving_unit = false;
		equity_perception_threshold = 0;
		net_salary_impact_on_equity = 0;
		savings_impact_on_equity = 0;
		unit_net_revenue = 0;
	}

	void economic_unit::set_costs( const expense_and_cost& costs )
	{
		applicable_costs = costs;
	}
	
	void economic_unit::set_revenues( const revenues_and_profits& revenues )
	{
		applicable_revenues = revenues;
	}

	currency_type economic_unit::get_unit_net_revenue()
	{
		return unit_net_revenue;
	}

	//Constructor for economy_manager
	economy_manager::economy_manager( const std::string& player )
	try {
		LOG("economy_manager::economy_manager(): For ", player );
		eco_variables = new economic_variables;
		player_name = player;

		public_welfare_funds = new public_welfare;
	}catch( std::exception& xa )
	{
		LOG_ERR("economy_manager::economy_manager() Exception catched, what: ",xa.what() );
	}

	economy_manager::~economy_manager()
	{
		LOG("economy_manager::~economy_manager(): Destroying the object");
		for( auto eu_elem : eu_container )
		{
			delete eu_elem;
		}
		if( eco_variables )
		{
			delete eco_variables;
		}
		if( public_welfare_funds )
		{
			delete public_welfare_funds;
		}
	}

	//Create a basic economic unit
	economic_unit* economy_manager::create_economic_unit( currency_type salary, game_wallet cash )
	try{
		ELOG("economy_manager::create_economic_unit(): New EU requested");
		economic_unit* eu = new economic_unit;
		eu->gross_salary = salary;
		eu->wallet = cash;
		eu->salary_cl = calculate_salary_class( salary );

		srand( time( nullptr ) + clock() );
		short tentative_nbr = 3; //Try three times to get a non-zero threshold value.
		do{
			eu->equity_perception_threshold = ( double )( rand() % 100 ) / 100; //Every EU have its own threshold
			if( eu->net_salary_impact_on_equity == 0 )
			{
				eu->net_salary_impact_on_equity = ( double )( rand() % 100 ) / 100;
			}
			if( eu->savings_impact_on_equity == 0 )
			{
				eu->savings_impact_on_equity = ( double )( rand() % 100 ) / 100;
			}
		}
		while( ( eu->equity_perception_threshold == 0 ) && ( tentative_nbr-- > 0 ) );
		if( eu->net_salary_impact_on_equity < 0.7 )
		{
			eu->net_salary_impact_on_equity = 0.7; //Not too low.
		}
		if( eu->savings_impact_on_equity < 0.5 )
		{
			eu->savings_impact_on_equity = 0.5;
		}
		ELOG("economy_manager::create_economic_unit(): New EU: Salary: ",salary,", Cash: ",cash.available_free_cash(),", Equity: ",eu->equity_perception_threshold,", SalIE: ",eu->net_salary_impact_on_equity,", SavIE: ",eu->savings_impact_on_equity );
		return eu;
	}catch( std::exception& xa )
	{
		LOG_ERR("economy_manager::create_economic_unit() Exception catched, what: ",xa.what() );
		return nullptr;
	}

	//Create a default economic unit
	economic_unit* economy_manager::create_economic_unit( salary_class sclass )
	{
		economic_unit* eu = nullptr;
		currency_type salary = get_basic_class_salary( sclass );
		if( salary > 0 )
		{
			game_wallet wallet( 0 );
			eu = create_economic_unit( salary, wallet );
		}
		return eu;
	}

	//Add the economic unit to the current pool
	bool economy_manager::add_economic_unit( economic_unit* eu )
	{
		bool verdict = false;
		if( eu )
		{
			std::lock_guard< std::mutex > lock( eu_container_mutex );
			ELOG("economy_manager::add_economic_unit(): Adding a new EU, family size: ",*eu->family_size,", gross_salary: ",eu->gross_salary,", wallet: ",eu->wallet.available_free_cash());
			eu_container.push_back( eu );
		}
		return verdict;
	}

	/*
	 * For each class the function return the salary that by default is assigned 
	 * to that class.
	 */
	currency_type economy_manager::get_basic_class_salary( salary_class sclass )
	{
		currency_type salary = 0;
		if( sclass >= 0 && sclass <= 5 )
		{
			salary = eco_variables->default_class_salary[ sclass ];
			//Make the salary little bit random
			srand( clock() );
			long num = rand();
			if( num % 2 ) 
			{
				num *= -1;
			}
			salary *= ( 1 + ( num % SALARY_CLASS_MIN_DELTA ) /  ( double ) 100 );
		}
		return salary;
	}

	inline
	salary_class economy_manager::calculate_salary_class( currency_type salary )
	{
		for( short cl = 0 ; cl < 6 ; cl++ )
		{
			if( salary < eco_variables->salary_class[ cl ] ) 
			{
				return salary_class( cl );
			}
		}
		LOG_ERR("economy_manager::calculate_salary_class(): Not able to find the salary class for: ", salary);
		return salary_class( 0 );
	}

	//This function returns the costs which belongs to the economic_unit
	currency_type economy_manager::apply_expense_and_cost( economic_unit* eu )
	{
		currency_type result = 0;
		result += eu->applicable_costs.living_cost;
		result += eu->applicable_costs.food_cost;
		result *= *eu->family_size;
		ELOG("economy_manager::apply_expense_and_cost(): Total cost for this unit ",result);
		return result;
	}

	void economy_manager::modify_tax_equity_perception( long double value )
	{
		value /= TAX_EQUITY_PERCEPTION_SENSIBILITY;
		eco_variables->salary_taxes_equity_perception += value;
		if( eco_variables->salary_taxes_equity_perception < 0 )
		{
			eco_variables->salary_taxes_equity_perception = 0;
		}
		else if( eco_variables->salary_taxes_equity_perception > 1 )
		{
			eco_variables->salary_taxes_equity_perception = 1;
		}
		ELOG("economy_manager::modify_tax_equity_perception(): required to make a change of :", value,", new value: ",eco_variables->salary_taxes_equity_perception );
	}

	inline
	currency_type economy_manager::calculate_gross_salary( economic_unit* eu )
	{
		return ( eu->gross_salary / 12 ) * *eu->family_size;
	}

	inline
	currency_type economy_manager::calculate_taxes_on_salary( economic_unit* eu )
	{
		return calculate_gross_salary( eu ) * eco_variables->salary_taxes[ eu->salary_cl ] / 100;
	}

	//The function will return the cost of the procedure for the player
	currency_type economy_manager::handle_starving_unit( economic_unit* eu , currency_type needed_money )
	{
		LOG("economy_manager::handle_starving_unit(): EU:",eu->eu_id,",Needed money:",needed_money);
		currency_type financed = 0;
		//If the free cash of the unit is not sufficient to cover the costs, the unit will be marked as poor!
		if( eu->wallet.available_free_cash() < needed_money || eu->starving_unit )
		{
			//Try to access the poor support program
			if( public_welfare_funds->is_welfare_available() &&
				( public_welfare_funds->can_access_poor_welfare_program( needed_money ) ) )
			{
				//The procedure have a cost, can the player pay for it?
				financed = public_welfare_funds->access_poor_subsidies_fund( needed_money );
				if( eu->starving_unit )
				{
					eu->starving_unit = false; //Is not starving anymore, since the welfare helped
				}
				//TODO: What to do next? Have the EU to pay back this money?
				ELOG("economy_manager::handle_starving_unit(): Financed by:",financed);
			}
			else
			{
				if( !eu->starving_unit )
				{
					eu->starving_unit = true;
				}
				eu->wallet.get_money_cash() = 0; //Pay what you can...
			}
		}
		else
		{
			remove_starving_state( eu ); // If the unit is starving, remove this state.
			ELOG("economy_manager::handle_starving_unit(): Paying with the savings, cash: ",eu->wallet.available_free_cash(),", expenses: ",needed_money);
			eu->wallet.get_money_cash() -= needed_money;
		}
		return financed * POOR_SUBSIDIES_ACCESS_COST;
	}

	//distribute the salary, return the amount of taxes collected.
	currency_type economy_manager::apply_salaries_and_collect_taxes( economic_unit* eu )
	{
		currency_type gross_salary = calculate_gross_salary( eu ); //Shall be a monthly value
		currency_type net_salary = 0, savings = 0;
		ELOG("economy_manager::apply_salaries_and_collect_taxes(): Unit gross salary: ", gross_salary,", family size: ", *eu->family_size );
		//Apply the taxe
		currency_type taxes = 0;
		if( gross_salary > 0 )
		{
			net_salary = gross_salary;
			taxes = calculate_taxes_on_salary( eu );
			net_salary -= taxes;
			//Apply the costs ( Like for the appartment rental ) , this will reduce the amount of net salary
			savings = net_salary - apply_expense_and_cost( eu );
			calculate_tax_equity_perception( gross_salary, net_salary, savings, eu );
			//If the saving is negative, then this economic_unit is too poor to effort is life style
			if( savings < 0 )
			{
				taxes -= handle_starving_unit( eu, std::abs( savings ) );
			}
			else
			{
				remove_starving_state( eu );
				eu->wallet.get_money_cash() += savings; //put the savings in the eu wallet.
			}
			ELOG("economy_manager::apply_salaries_and_collect_taxes(): New salary_taxes_equity_perception: ",eco_variables->salary_taxes_equity_perception, ", Total savings for this unit: ",eu->wallet.available_free_cash() );
		}
		return taxes;
	}

	void economy_manager::remove_starving_state( economic_unit* eu )
	{
		if( eu->starving_unit )
		{
			eu->starving_unit = false;
		}
	}

	void economy_manager::recalculate_amount_of_starving_unit()
	{
		eco_variables->amount_of_starving_unit = 0;
		for( auto elem : eu_container )
		{
			if( elem->is_starving() )
			{
				eco_variables->amount_of_starving_unit += elem->get_family_size();
			}
		}
	}

	//The net_salary_impact_on_equity and savings_impact_on_equity values are impacted for example by the total amount of
	//savings the unit have. More money means less negative impact on temporary low saving ratio
	void economy_manager::correct_savings_and_salary_impact_on_equity( economic_unit* eu, currency_type net_salary, currency_type savings )
	{
		currency_type total_savings = eu->wallet.available_free_cash();
		if( total_savings == 0 )
		{
			total_savings = 100;
		}
		auto calculate_impact = [&]( currency_type refer, currency_type base ) 
		{
			double impact = ( refer + 0.5 ) / ( refer + base );
			if( refer < 0 && impact > 0)
			{
				impact *= -1;
			}
			return impact;
		};
		auto check_boundaries = []( double val )
		{
			return ( val >= 0 && val <=1 ) ? val : ( val < 0 ? 0 : 1 );
		};
		eu->savings_impact_on_equity += calculate_impact( savings , total_savings );
		eu->savings_impact_on_equity = check_boundaries( eu->savings_impact_on_equity );
		eu->net_salary_impact_on_equity += calculate_impact( net_salary , eu->gross_salary );
		eu->net_salary_impact_on_equity = check_boundaries( eu->net_salary_impact_on_equity );
		LOG("economy_manager::correct_savings_and_salary_impact_on_equit(): EU:",eu->eu_id,", netS:",net_salary,",Savings:",savings,",TotalSav:",total_savings,", SavImpOnEq:",eu->savings_impact_on_equity,", SalImpOnEq:",eu->net_salary_impact_on_equity);
	}

	void economy_manager::calculate_tax_equity_perception( currency_type gross_salary, currency_type net_salary, currency_type savings, economic_unit* eu )
	{
		correct_savings_and_salary_impact_on_equity( eu, net_salary, savings );
		//Modify th perception of the equity for the tax level and the living prices,
		//It depend also on the amount of money the eu is able to save
		long total_fees = gross_salary - ( net_salary * eu->net_salary_impact_on_equity ) -
						 ( savings * eu->savings_impact_on_equity );

		long double perception = ( double ) total_fees / gross_salary; //1=very bad 0=very good
		ELOG("economy_manager::calculate_tax_equity_perception(): Gross S: ",gross_salary,", Net S: ",net_salary,", Savings: ",savings, ", UE Equity: ", eu->equity_perception_threshold,", Perceived Equity: ",perception);
	
		/*
		 * A threshold is defined, is perception is below the threshold then a positive impact on the equity perception is visible
		 * otherwise, the eu impact negatively on the equity perception for this economy.
		 * Generally speaking if an eu have a threshold of 0.5, the he whishes to have a total_fees / gross_salary ratio
		 * always below 0.5, like 0.2. 
		 */
		if( perception < eu->equity_perception_threshold )
		{
			//Good, this ue will impact positively the equity perception level.
			perception *= ( ( long double ) 1 / ( eu_container.size() + 1 ) );
			modify_tax_equity_perception( ( perception < 0 ? perception : perception * -1 ) );
		}
		else
		{
			//Not good!
			perception *= ( ( long double ) 1 / ( eu_container.size() + 1 ) );
			modify_tax_equity_perception( std::abs( perception ) ); 
		}
	}

	currency_type economy_manager::get_revenue_from_eu( economic_unit* eu )
	{
		currency_type total = 0;
		total += eu->applicable_revenues.revenue_from_rental * *eu->family_size;
		total += eu->applicable_revenues.revenue_from_selling_food * *eu->family_size;
		return total;
	}

	//The function review_economy check and update the economy status
	//calculate and distribute the salaries, apply the taxing rules ecc
	void economy_manager::review_economy()
	{
		LOG("economy_manager::review_economy(): EU count: ",eu_container.size());
		if( eu_container.empty() ) 
		{
			return; 
		}
		currency_type collected_money = 0;
		public_welfare_funds->zeroes_poor_subsidies_counter(); //Each review has it own statistics.
		eco_variables->total_eu_population = 0;
		for( auto eu : eu_container )
		{
			//Refresh the population count
			eco_variables->total_eu_population += eu->get_family_size();
			//Calculate the eu revenue
			eu->unit_net_revenue = 0;
			if( eu->get_family_size() > 0 )
			{
				//Apply the eu revenues
				eu->unit_net_revenue = get_revenue_from_eu( eu );
				//Distribute the salary and collect the taxes
				eu->unit_net_revenue += apply_salaries_and_collect_taxes( eu ); 
				//If for this EU maintanance costs are present, apply those costs
			}
			if( eu->applicable_costs.player_maintanance_cost > 0 )
			{
				ELOG("economy_manager::review_economy(): Applying costs to ", player_name,", for: ",eu->applicable_costs.player_maintanance_cost);
				eu->unit_net_revenue -= eu->applicable_costs.player_maintanance_cost;
			}
			ELOG("economy_manager::review_economy(): EU Net Rev.: ", eu->unit_net_revenue );
			collected_money += eu->unit_net_revenue;
		}
		recalculate_amount_of_starving_unit();
		//Add the collected taxes to the player wallet after the various funds detraction
		collected_money = public_welfare_funds->walfare_distribute_revenue( collected_money );
		player_wallet.get_money_cash() += collected_money;
		ELOG("economy_manager::review_economy(): Amount of money collected for ",player_name,": ",collected_money,", Saldo: ",player_wallet.get_money_cash() );
	}

	const economic_variables* economy_manager::get_the_economic_variables( )
	{
		return eco_variables;
	}

	currency_type economy_manager::player_available_money()
	{
		return player_wallet.available_free_cash();
	}

	currency_type economy_manager::player_apply_cost( currency_type value )
	{
		if( value <= player_wallet.available_free_cash() )
		{
			player_wallet.get_money_cash() -= value;
			return player_wallet.available_free_cash();
		}
		return -1; // Not enoght resources
	}

	currency_type economy_manager::player_add_cash( currency_type value )
	{
		if( value > 0 )
		{
			player_wallet.get_money_cash() += value;
		}
		return player_wallet.available_free_cash();
	}

	public_welfare* economy_manager::get_public_walfare_funds()
	{
		return public_welfare_funds;
	}

	double economy_manager::set_salary_tax_level( salary_class sc , double new_level )
	{
		if( new_level < 0 || new_level > 100 )
			return -1;
		double current_level = eco_variables->salary_taxes[ sc ];
		LOG("economy_manager::set_salary_tax_level(): From:",current_level,", To:",new_level);
		eco_variables->salary_taxes[ sc ] = new_level;
		return current_level;
	}
}

