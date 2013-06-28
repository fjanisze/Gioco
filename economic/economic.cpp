#include "economic.hpp"



namespace job_market
{

	const job_descriptor civil_unemployed_jon_level0 = { 0 , "Unemployed", 0 }; //The 0 ID shall be always for the unemployed
	const job_descriptor civil_scullion_job_level0 = { 1 , "Scullion", 19500 };
	const job_descriptor civil_hodman_job_level0 = { 2 , "Hodman", 21000 };
	const job_descriptor civil_office_job_level1 = { 3 , "Office employee" , 35000 };

	job_entity::job_entity( const job_descriptor* job ) : descriptor( job )
	{
		amount_of_workplaces = 0;
		gross_salary = 0;
		amount_of_free_workplaces = 0;
	}

	job_market_manager::job_market_manager()
	{
		LOG("job_market_manager::job_market_manager(): Creating a new instance ");
	}

	job_market_manager::~job_market_manager( )
	{
	}

	//Create a new basic job entity
	job_entity* job_market_manager::create_new_job_entity( const job_descriptor* job )
	try{
		LOG("job_market_manager::create_new_job_entity(): Job: ",job->name );
		job_entity* entity = new job_entity( job );
		entity->descriptor = job;
		entity->gross_salary = job->base_gross_salary;

		//Randomize a little the salary
		srand( clock() );
		long num = rand();
		if( num % 2 ) 
		{
			num *= -1;
		}
		entity->gross_salary *= ( 1 + ( num % SALARY_CLASS_MIN_DELTA ) /  ( double ) 100 );

		return entity;
	}catch( std::exception& xa )
	{
		LOG_ERR("job_market_manager::create_new_job_entity(): Exception catched, what: ",xa.what() );
		return nullptr;
	}
	
	//Add a new job to the job market
	long job_market_manager::register_job_entity( job_entity* job , long workplaces )
	{
		if( job )
		{
			job->amount_of_workplaces = workplaces;
			job->amount_of_free_workplaces = workplaces;
			std::lock_guard< std::mutex > lock( access_mutex );
			available_jobs.push_back( job );
			ELOG("job_market_manager::register_job_entity(): Amount of jobs: ", available_jobs.size() );
		}
		return available_jobs.size();
	}

	//Return the job id for the provided unit
	long job_market_manager::get_job_id( economic_unit* eu )
	{
		return eu->get_job()->descriptor->job_id;
	}

	//Look for a job which fit the amount of workplaces needed
	//The job is immediatly assigned
	job_entity* job_market_manager::find_job_entity( long workplaces_needed )
	{
		job_entity* job = nullptr;
		std::lock_guard< std::mutex > lock( access_mutex );
		for( auto elem : available_jobs )
		{
			if( elem->amount_of_free_workplaces >= workplaces_needed )
			{
				job = elem;
				break;
			}
		}
		return job;
	}

	bool job_market_manager::is_unemployed( economic_unit* eu )
	{
		if( eu->get_job() == nullptr )
		{
			return true;
		}
		return eu->get_job()->descriptor->job_id == civil_unemployed_jon_level0.job_id;
	}

	//And return the amount of free workplaces
	long job_market_manager::remove_from_workplace_distr( job_entity* job, long eu_id )
	{
		ELOG("job_market_manager::remove_from_workplace_dist(): EU:",eu_id,",JobID:",job->descriptor->job_id);
		if( job->workplaces_distribution.find( eu_id ) != job->workplaces_distribution.end() )
		{
			job->amount_of_free_workplaces += job->workplaces_distribution[ eu_id ];
			job->workplaces_distribution.erase( eu_id );
		}
		return job->amount_of_free_workplaces;
	}

	//This unit is not doing that job anymore
	void job_market_manager::leave_the_job( economic_unit* eu )
	{
		ELOG("job_market_manager::leave_the_job(): EU:",eu->get_id()," is leaving the job:",get_job_id( eu ) );
		if( !is_unemployed( eu ) ) //A unemployes cannot leave is job.. since have no job..
		{
			remove_from_workplace_distr( eu->get_job(), eu->get_id() );
			//The unit now should be unemployed
			eu->set_job( create_new_job_entity( &civil_unemployed_jon_level0 ) );
		}
	}

	bool job_market_manager::update_workplace_distribution( job_entity* job, long eu_id, long amount )
	{
		ELOG("job_market_manager::update_workplace_distribution(): Job:",job->descriptor->job_id,",EU:",eu_id,",Amount:",amount);
		if( job->workplaces_distribution.find( eu_id ) != job->workplaces_distribution.end() )
		{
			//Check if this change is applicable for this specific eu
			long eu_workforce = job->amount_of_workplaces - job->amount_of_free_workplaces;
			eu_workforce += amount;
			if( eu_workforce < 0 || eu_workforce > job->amount_of_workplaces )
			{
				ELOG("job_market_manager::update_workplace_distribution(): Required change is out of range:",eu_workforce,",max value:",job->amount_of_workplaces);
				return false;
			}
			//Changing the values
			job->amount_of_free_workplaces -= amount;
			job->workplaces_distribution[ eu_id ] += amount;
			if( job->workplaces_distribution[ eu_id ] == 0 )
			{
				job->workplaces_distribution.erase( eu_id );
			}
			return true;
		}
		else if( amount > 0 )
		{
			job->workplaces_distribution[ eu_id ] = amount;
			return true;
		}
		return false;
	}

	//Return the amount of eu employed
	long job_market_manager::amount_of_employed_people( job_entity* job, long eu_id )
	{
		if( job == nullptr )
		{
			return 0;
		}
		if( job->workplaces_distribution.find( eu_id ) != job->workplaces_distribution.end() )
		{
			return job->workplaces_distribution[ eu_id ];
		}
		return 0;
	}

	//This eu is applying for this job
	long job_market_manager::apply_for_the_job( job_entity* job , long amount_of_workplaces, economic_unit* eu )
	{
		ELOG("job_market_manager::apply_for_the_job(): EU: ",eu->get_id()," is applying for the job:",job->descriptor->job_id,", needed workplaces:",amount_of_workplaces,", free workplaces:",job->amount_of_free_workplaces);
		long uneployed = amount_of_workplaces;
		if( job->amount_of_free_workplaces >= amount_of_workplaces ) //Are the workspaces enough?
		{
			if( eu->get_job() != nullptr )
			{
				if( !is_unemployed( eu ) )
				{
					leave_the_job( eu );
				}
				//Now for sure is an unemployed unit
				delete eu->get_job();
			}
			job->amount_of_free_workplaces -= amount_of_workplaces;
			uneployed = 0;
			eu->set_job( job );
			eu->set_salary_cl( calculate_salary_class( job->gross_salary ) );
			//Check if the workforce count have to be updated
			if( !update_workplace_distribution( job, eu->get_id() , amount_of_workplaces ) )
			{
				//Nothing to update, is a fresh employer
				job->workplaces_distribution[ eu->get_id() ] = amount_of_workplaces;
			}
		}
		return uneployed;
	}

	salary_class job_market_manager::calculate_salary_class( currency_type salary )
	{
		for( short cl = 0 ; cl < 6 ; cl++ )
		{
			if( salary < static_cast< economy_manager* >(this)->get_the_economic_variables()->salary_class[ cl ] ) 
			{
				return salary_class( cl );
			}
		}
		LOG_ERR("job_market_manager::calculate_salary_class(): Not able to find the salary class for: ", salary );
		return salary_class( 0 );
	}

	//Look and apply for a job. If the unit have already a job, it try to increase the amount
	//of workplaces used, if is not possible, leave the job and look for a new one
	job_entity* job_market_manager::look_for_a_job( economic_unit* eu , const string& eu_city )
	{
		job_entity* job = nullptr;
		long eu_id = eu->get_id();
		long required_workspaces = eu->get_family_size() - amount_of_employed_people( eu->get_job(), eu_id );
		LOG("job_market_manager::look_for_a_job(): ID:",eu_id,", Workplaces needed:",required_workspaces );
		//Should be positive
		if( required_workspaces > 0 )
		{
			//Look for a job with fit the family size
			for( auto elem : available_jobs )
			{
				//Each EU will search for jobs in is own city
				if( eu_city != elem->city )
				{
					LOG_WARN("job_market_manager::look_for_a_job(): EU:",eu_id,", cannot work in ",elem->city,", since is from:", eu_city );
					continue;
				}
				//Try to apply for the job
				if( apply_for_the_job( elem, required_workspaces, eu ) == 0 )
				{
					job = eu->get_job(); //Ok
					break;
				}
			}
		}
		//Any job?
		if( job == nullptr )
		{
			//No.. set as unemployed
			job = create_new_job_entity( &civil_unemployed_jon_level0  ); 
			eu->set_job( job );
			eu->set_salary_cl( salary_class( 0 ) );
		}
		ELOG("job_market_manager::look_for_a_job(): EU:",eu_id,", Job found:",get_job_id( eu ) );
		return job;
	}

	/*
	 * This function loop throught all the eu, check if they have a job, if not, 
	 * enable the job searching algorithm; 
	 */
	long job_market_manager::update_uneployment_level( eu_container& eu  )
	{
		LOG("job_market_manager::update_uneployment_level(): Num of EU:",eu.count() );
		long unemployment = 0;
		economic_unit* elem = eu.get_first_eu();
		string eu_city;
		while( elem != nullptr )
		{
			eu_city = eu.get_eu_city( elem );
			if( is_unemployed( elem ) )
			{
				//Look for a job
				job_entity* job = look_for_a_job( elem , eu_city );
				if( is_unemployed( elem ) ) //Still unemployed?
				{
					unemployment += elem->get_family_size();
				}
			}
			else
			{
				//The family size has changed?
				long workplaces_difference = elem->get_family_size();
				workplaces_difference -= amount_of_employed_people( elem->get_job(), elem->get_id() );
				if( workplaces_difference != 0 )
				{
					ELOG("job_market_manager::update_uneployment_level(): workplaces difference:",workplaces_difference);
					if( !update_workplace_distribution( elem->get_job(), elem->get_id(), workplaces_difference ) )
					{
						//Not possible to increase/decrease the amount of workplaces
						//'max' or 0 reached, the eu will leave the job and start to search
						//a new one
						leave_the_job( elem );
						look_for_a_job( elem , eu_city );
					}
				}
				if( amount_of_employed_people( elem->get_job(), elem->get_id() ) == 0 )
				{
					//No one is working here.. leave the job
					LOG("job_market_manager::update_uneployment_level(): This unit have a job, but no workplaces consumed..");
					leave_the_job( elem );
				}
			}
			elem = eu.get_next_eu();
		}
		ELOG("job_market_manager::update_uneployment_level(): Uneployment: ", unemployment);
		return unemployment;
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
		salary_taxes[0] = 19; //Lower lowel, for the poors.
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

	//////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for economic_unit
	//
	//
	//////////////////////////////////////////////////////////////////

	long economic_unit::eu_next_unique_id = 1;

	economic_unit::economic_unit()
	{
		eu_id = eu_next_unique_id++;
		//At the very beginning the eu has nothing.
		wallet = nullptr;
		eu_bank = nullptr;
		family_size = nullptr;
		starving_unit = false;
		equity_perception_threshold = 0;
		net_salary_impact_on_equity = 0;
		savings_impact_on_equity = 0;
		unit_net_revenue = 0;
		have_payed_rental_price = true;
		job = nullptr;
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
	
	currency_type economic_unit::get_gross_salary()
	{	
		if( job )
		{
			return job->gross_salary;
		}
		return 0;	
	}

	long economic_unit::get_id() const
	{
		return eu_id;
	}

	//Return true if this unit is unemployed
	bool economic_unit::is_unemployed()
	{
		return ( job == nullptr ) || ( job->descriptor->job_id == 0 );
	}

	bool economic_unit::look_for_a_job()
	{
		if( is_unemployed() )
		{
			job_entity* job;
		}
	}

	//economy_info structure
	void economy_info::clear()
	{
		total_taxes_net_revenue = 0;
		total_revenue_from_building_rental = 0;
		total_building_maintanance_cost = 0;
		total_welfare_funding = 0;
		total_net_income = 0;
	}

	////////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for eu_container
	//
	//
	//////////////////////////////////////////////////////////////////

	eu_container::eu_container() : head( nullptr ), tail( nullptr ), eu_count( 0 ) , eu_pos( nullptr )
	{	}

	//Create an empty element
	eu_node* eu_container::create() throw( )
	try{
		eu_node* node = new eu_node;
		return node;
	}catch( ... )
	{
		return nullptr;
	}

	//Insert a new element in the container
	eu_node* eu_container::add( economic_unit* eu, const std::string& city )
	{
		eu_node* node = create();
		if( node != nullptr )
		{
			node->eu = eu;
			node->city = city;
			//Is the list empty?
			if( tail == nullptr )
			{
				head = tail = node;
			}
			else
			{
				//Add the element
				tail->next = node;
				node->prev = tail;
				tail = node;
			}
			++eu_count;
			//Update the city link
			if( head_same_city.find( city ) == head_same_city.end() )
			{
				//First time this city is appearing
				head_same_city[ city ] = node;
			}
			else
			{
				//Add to the list
				eu_node* city_tail = head_same_city[ city ];
				while( city_tail->next_same_city != nullptr )
				{
					city_tail = city_tail->next_same_city;
				}
				city_tail->next_same_city = tail;
				tail->prev_same_city = city_tail;
				city_tail = tail;
			}
		}
		return node;
	}

	//The next functions are used to find and extract an element from the list
	economic_unit* eu_container::get_first_eu()
	{
		eu_pos = head;
		if( eu_pos == nullptr )
		{
			return nullptr;
		}
		return head->eu;
	}

	economic_unit* eu_container::get_next_eu()
	{
		if( eu_pos->next != nullptr )
		{
			eu_pos = eu_pos->next;
			return eu_pos->eu;
		}
		return nullptr;
	}

	//The next functions are used to move throught the eu in the same city
	economic_unit* eu_container::get_first_eu_city( const std::string& city )
	{
		if( head_same_city.find( city ) != head_same_city.end() )
		{
			eu_pos = head_same_city[ city ];
			return eu_pos->eu;
		}
		return nullptr;
	}

	economic_unit* eu_container::get_next_eu_city()
	{
		if( eu_pos->next_same_city != nullptr )
		{
			eu_pos = eu_pos->next_same_city;
			return eu_pos->eu;
		}
		return nullptr;
	}

	//Return the city where this eu is located
	string eu_container::get_eu_city( const economic_unit* eu )
	{
		string city;
		eu_node* pos = head;
		long eu_id = eu->get_id();
		while( pos != nullptr )
		{
			if( pos->eu->get_id() == eu_id )
			{
				return pos->city;
			}
			pos = pos->next;
		}
		return city;
	}

	////////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for economy_manager
	//
	//
	//////////////////////////////////////////////////////////////////

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
		economic_unit* eu = eu_all.get_first_eu();
		while( eu != nullptr )
		{
			delete eu;
			eu = eu_all.get_next_eu();
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
	economic_unit* economy_manager::create_economic_unit( job_entity* job, eu_type_t eu_type )
	try{
		LOG("economy_manager::create_economic_unit(): New EU requested, job: ", job != nullptr );
		economic_unit* eu = new economic_unit;
		//Create the bank account for this unit
		eu->eu_bank = banking.find_bank();
		assert( eu->eu_bank != nullptr );
		bank_account* acc = eu->eu_bank->open_new_account( eu->eu_id );
		assert( acc != nullptr );
		eu->wallet = acc;
		//Different creation procedure for different type of units
		if( eu_type == eu_type_t::population_unit )
		{
			if( job == nullptr )
			{
				//Create a default job: unemployed!!
				eu->job = create_new_job_entity( &civil_unemployed_jon_level0 );
			}
			else
			{
				eu->job = job;
			}
			calculate_impact_on_equity_factors( eu );
			ELOG("economy_manager::create_economic_unit(): New EU:",eu->eu_id,",Job: ", eu->job->descriptor->name ,", Equity: ",eu->equity_perception_threshold,", SalIE: ",eu->net_salary_impact_on_equity,", SavIE: ",eu->savings_impact_on_equity );
		}
		else if( eu_type == eu_type_t::corporate_unit )
		{
			ELOG("economy_manager::create_economic_unit(): New EU:",eu->eu_id,", is a corporate unit");
		}
		return eu;
	}catch( std::exception& xa )
	{
		LOG_ERR("economy_manager::create_economic_unit() Exception catched, what: ",xa.what() );
		return nullptr;
	}


	void economy_manager::calculate_impact_on_equity_factors( economic_unit* eu )
	{
		LOG("economy_manager::calculate_impact_on_equity_factors(): EU:",eu->eu_id );
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
	}

	//Add the economic unit to the current pool
	bool economy_manager::add_economic_unit( economic_unit* eu , const string& city )
	{
		bool verdict = false;
		if( eu )
		{
			std::lock_guard< std::mutex > lock( eu_container_mutex );
			currency_type gross_salary = 0;
			if( eu->eu_type == eu_type_t::population_unit )
			{
				assert( eu->job != nullptr );
				gross_salary = eu->job->gross_salary;
				ELOG("economy_manager::add_economic_unit(): Adding EU:",eu->get_id(),", family size: ",*eu->family_size,", gross_salary: ",gross_salary,", wallet: ",eu->wallet->get_balance() );
			}
			else if( eu->eu_type == eu_type_t::corporate_unit )
			{
				ELOG("economy_manager::add_economic_unit(): Adding EU:",eu->get_id(),", which is a corporate type");
			}	
			eu_all.add( eu , city );
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
		return ( eu->job->gross_salary / 12 ) * *eu->family_size;
	}

	inline
	currency_type economy_manager::calculate_taxes_on_salary( economic_unit* eu )
	{
		return calculate_gross_salary( eu ) * ( eco_variables->salary_taxes[ eu->salary_cl ] / 100 );
	}

	//The function will return the cost of the procedure for the player
	currency_type economy_manager::handle_starving_unit( economic_unit* eu , currency_type needed_money )
	{
		LOG("economy_manager::handle_starving_unit(): EU:",eu->eu_id,",Needed money:",needed_money);
		currency_type financed = 0;
		//If the free cash of the unit is not sufficient to cover the costs, the unit will be marked as poor!
		if( eu->wallet->get_balance() < needed_money || eu->starving_unit )
		{
			//Try to access the poor support program
			if( public_welfare_funds->is_welfare_available() &&
				( public_welfare_funds->can_access_poor_welfare_program( needed_money ) ) )
			{
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
			}
		}
		else
		{
			remove_starving_state( eu ); // If the unit is starving, remove this state.
			ELOG("economy_manager::handle_starving_unit(): Paying with the savings, cash: ",eu->wallet->get_balance(),", expenses: ",needed_money);
			eu->wallet->withdraw( needed_money );
		}
		last_cycle_info.total_welfare_funding += financed;
		return financed * POOR_SUBSIDIES_ACCESS_COST; //The procedure have a cost, can the player pay for it?
	}

	//distribute the salary, return the amount of taxes and rental revenue collected.
	currency_type economy_manager::get_revenue_from_eu( economic_unit* eu )
	{
		currency_type gross_salary = calculate_gross_salary( eu ); //Shall be a monthly value
		ELOG("economy_manager::get_revenue_from_eu(): EU:",eu->get_id(),", Gross salary: ", gross_salary,", family size: ", *eu->family_size );
		//Increase the counter about the job duration
		
		//Apply the taxe
		currency_type taxes = 0;
		currency_type revenue_from_rental = 0;
		bool this_unit_is_starving = false;
		currency_type needed_money = 0;
		if( gross_salary > 0 )
		{
			currency_type net_salary = 0, savings = 0;
			net_salary = gross_salary;
			taxes = calculate_taxes_on_salary( eu );
			net_salary -= taxes;
			//Apply the costs ( Like for the appartment rental ) , this will reduce the amount of net salary
			savings = net_salary - apply_expense_and_cost( eu );
			//Calculate the revenue from the rental
			revenue_from_rental = eu->applicable_revenues.revenue_from_rental * *eu->family_size;
			calculate_tax_equity_perception( gross_salary, net_salary, savings, eu );
			//If the saving is negative, then this economic_unit is too poor to effort is life style
			if( savings < 0 )
			{
				//Since is very poor refund the taxes 
				eu->wallet->deposit( taxes );
				taxes = 0;//No taxes since were refunded
				this_unit_is_starving = true;
				needed_money = std::abs( savings );
			}
			else
			{
				remove_starving_state( eu );
				eu->wallet->deposit( savings ); //put the savings in the eu wallet.
			}
			ELOG("economy_manager::get_revenue_from_eu(): New salary_taxes_equity_perception: ",eco_variables->salary_taxes_equity_perception, ", Total savings for this unit: ",eu->wallet->get_balance() );
		}
		else
		{
			//This unit does not have a salary, most probably is unemployed
			this_unit_is_starving = true;
			needed_money = apply_expense_and_cost( eu ); 
		}
		if( this_unit_is_starving )
		{
			taxes -= handle_starving_unit( eu, needed_money ); //The function return the financing cost, which impact negatively on the revenue 
		}
		eu->have_payed_rental_price = ( revenue_from_rental > 0 );
		//Update the economy statistic
		last_cycle_info.total_taxes_net_revenue += taxes;
		last_cycle_info.total_revenue_from_building_rental += revenue_from_rental;
		return taxes + revenue_from_rental;
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
		economic_unit* elem = eu_all.get_first_eu();
		while( elem != nullptr )
		{
			if( elem->is_starving() )
			{
				eco_variables->amount_of_starving_unit += elem->get_family_size();
			}
			elem = eu_all.get_next_eu();
		}
	}

	//The net_salary_impact_on_equity and savings_impact_on_equity values are impacted for example by the total amount of
	//savings the unit have. More money means less negative impact on temporary low saving ratio
	void economy_manager::correct_savings_and_salary_impact_on_equity( economic_unit* eu, currency_type net_salary, currency_type savings )
	{
		currency_type total_savings = eu->wallet->get_balance(), gross_salary = eu->job->gross_salary;
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

		eu->net_salary_impact_on_equity += calculate_impact( net_salary , gross_salary );
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
			perception *= ( ( long double ) 1 / ( eu_all.count() + 1 ) );
			modify_tax_equity_perception( ( perception < 0 ? perception : perception * -1 ) );
		}
		else
		{
			//Not good!
			perception *= ( ( long double ) 1 / ( eu_all.count() + 1 ) );
			modify_tax_equity_perception( std::abs( perception ) ); 
		}
	}

	/*
	 * This function collects the money from the taxes and other fees
	 */
	currency_type economy_manager::collect_money()
	{
		LOG("economy_manager::collect_money(): EU count: ",eu_all.count() );
		if( eu_all.empty() ) 
		{
			return 0; 
		}
		currency_type collected_money = 0;
		public_welfare_funds->zeroes_poor_subsidies_counter(); //Each review has it own statistics.
		eco_variables->total_eu_population = 0;
		economic_unit* eu = eu_all.get_first_eu();
		while( eu != nullptr )
		{
			//Refresh the population count
			eco_variables->total_eu_population += eu->get_family_size();
			//Calculate the eu revenue
			eu->unit_net_revenue = 0;
			if( eu->get_family_size() > 0 )
			{
				eu->unit_net_revenue = get_revenue_from_eu( eu );
			}
			if( eu->applicable_costs.player_maintanance_cost > 0 )
			{
				ELOG("economy_manager::collect_money(): EU:",eu->get_id(),", Mnt:",eu->applicable_costs.player_maintanance_cost);
				eu->unit_net_revenue -= eu->applicable_costs.player_maintanance_cost;
				last_cycle_info.total_building_maintanance_cost += eu->applicable_costs.player_maintanance_cost;
			}
			ELOG("economy_manager::collect_money(): EU:",eu->get_id(),",Net Rev.: ", eu->unit_net_revenue );
			collected_money += eu->unit_net_revenue;
			eu = eu_all.get_next_eu();
		}
		recalculate_amount_of_starving_unit();
		//Add the collected taxes to the player wallet after the various funds detraction
		collected_money = public_welfare_funds->walfare_distribute_revenue( collected_money );
		player_wallet.get_money_cash() += collected_money;
		ELOG("economy_manager::collect_money(): Amount of money collected for ",player_name,": ",collected_money,", Saldo: ",player_wallet.get_money_cash() );
		return collected_money;
	}	

	//The function review_economy check and update the economy status
	//calculate and distribute the salaries, apply the taxing rules ecc
	void economy_manager::review_economy()
	{
		last_cycle_info.clear();
		//first step is to update the job market status
		long uneployment_level = update_uneployment_level( eu_all );
		//Then collect the money
		currency_type collected_money = collect_money();
		last_cycle_info.total_net_income = collected_money;
		//Once a while the function will print a dump of the economic unit
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

	economy_info economy_manager::get_cycle_statistic()
	{
		return last_cycle_info;
	}

	banking_manager* economy_manager::get_banking_manager()
	{
		return &banking;
	}
}

