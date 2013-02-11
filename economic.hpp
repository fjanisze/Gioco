#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#ifndef ECONOMIC_HPP
#define ECONOMIC_HPP

#include "logging/logger.hpp"
#include "common_structures.hpp"
#include <vector>
#include <map>
#include <exception>
#include <mutex>
#include <cmath>
#include <stdlib.h>
#include <ctime>
#include <cassert>
using namespace std;

typedef long long mlong;

#define TAX_EQUITY_PERCEPTION_SENSIBILITY 1000 //Not 0!! >=1, higher value then lower sensibility
#define POOR_SUBSIDIES_ACCESS_COST 0.15 //15% of the provided sum
#define SALARY_CLASS_MIN_DELTA 20 //Used in get_basic_salary_class

namespace finance
{
	typedef long double longd;
	typedef long long mlong;
	typedef long double currency_type;

	//The game_wallet structure contain the information about the player wealth
	class game_wallet
	{
		currency_type money_cash; //Amount of money available in the form of cash
	public:
		game_wallet() : money_cash( 0 )
		{	}

		game_wallet( const currency_type& value );
		game_wallet( const game_wallet& wallet );

		game_wallet& operator=( const game_wallet& ) = default;
		game_wallet& operator=( const currency_type& value );

		currency_type available_free_cash();
		currency_type& get_money_cash()
		{	return money_cash;	}
	};
}

namespace economics
{
	class economic_unit;
}

namespace job_market
{
	using namespace finance;
	using namespace economics;

	//Basic information about a job
	struct job_descriptor
	{
		const long job_id;

		string name;



		currency_type base_gross_salary;
	};

	//A serie of standard descriptor
	extern const job_descriptor civil_unemployed_jon_level0 ;
	extern const job_descriptor civil_scullion_job_level0;
	extern const job_descriptor civil_office_job_level1;

	//Each EU which have a job should have a job_entity
	struct job_entity
	{
		const job_descriptor* descriptor;
		string employee_name;//Who is your employer?
		currency_type gross_salary; //This may differ from the base_gross_salary, since the salary change during the time
	
		long amount_of_workplaces,
		     amount_of_free_workplaces;

		map< long , long > workplaces_distribution;//Which eu works here? How much of people?

		job_entity( const job_descriptor* job );
	};

	class job_market_manager
	{
		std::mutex access_mutex;
		std::vector< job_entity* > available_jobs;
		job_entity* find_job_entity( long workplaces_needed ); 
		long apply_for_the_job( job_entity* job, long amount_of_workplaces,  economic_unit* eu );
		void leave_the_job( economic_unit* eu );
		bool update_workplace_distribution( job_entity* job, long eu_id, long amount );
		long amount_of_employed_people( job_entity* job, long eu_id );
	public:
		job_market_manager();
		~job_market_manager();
		job_entity* create_new_job_entity( const job_descriptor* job );
		long register_job_entity( job_entity* job , long workplaces );
		job_entity* look_for_a_job( economic_unit* eu );
		long get_job_id( economic_unit* eu );
		bool is_unemployed( economic_unit* eu );
	};
};

namespace economics
{
	using namespace finance;
	using namespace job_market;

	typedef enum salary_class
	{
		class_0 = 0, //Very poor
		class_1,
		class_2,
		class_3,
		class_4,
		class_5 //Very rich
	} salary_class;

	class public_welfare
	{
		double poor_subsidies_budget; // Amount of net revenue ( in % ) which will flow to the poor subsidies fund
		currency_type poor_subsidies_fund; //Here the money are colleted. The starving units may ask to be included in the help program.
		long poor_subsidies_request_counter;
		currency_type poor_subsidies_total_funding;//Amount of money distributed
		bool is_available; //Is true if the public welfare is available for the player
	public:
		currency_type walfare_distribute_revenue( currency_type revenue ); 
		currency_type get_poor_subsidies_funds();
		bool can_access_poor_welfare_program( currency_type needed_money );
		bool is_poor_welfare_program_active();
		currency_type access_poor_subsidies_fund( currency_type needed_money );
		currency_type get_poor_subsidies_total_funding();
		void zeroes_poor_subsidies_counter();
		long get_poor_subsidies_counter();
		public_welfare();
		void set_welfare_availability( bool status );
		bool is_welfare_available();
	};

	/*
	 * This structure contain definitions for variable which allow to put every eu in a
	 * certain class, with specific taxes ecc. Also other variables which impact globally
	 * the economy are defined here
	 */
	struct economic_variables
	{
		currency_type salary_class[6]; //Higher the class then richest is the eu. The value is expressed in gross value
		float         salary_taxes[6]; //6 Different tax level are defined, higher is your salary then higher may be the taxation
		currency_type default_class_salary[6]; //This is the default salary which is assigned to each class.
		mlong         amount_of_starving_unit; //Those people are not able to pay its bill or rental price
		double        salary_taxes_equity_perception; //A value near to 1 means a very negative perception of the current tax level, most probably is too high
		mlong         total_eu_population;
		economic_variables();
	};

	class economy_manager;

	//This structure holds the information about the possible cost which belongs to an economic_unit
	struct expense_and_cost
	{
		currency_type living_cost; //Those cost comes from the price payed for the appartment rental for example
		currency_type food_cost; //Expenses for buying food
		currency_type player_maintanance_cost; //Describe the cost which belong to the player for this unit.
		expense_and_cost() : living_cost( 0 ) , food_cost( 500 ), player_maintanance_cost( 0 ) //Standard food expense for everyone
		{	}
	};

	//This structure contains the the values of the revenues or profits for the player
	//from this eu
	struct revenues_and_profits
	{
		currency_type revenue_from_rental; 
		revenues_and_profits() : revenue_from_rental( 0 ) 
		{	}
	};

	/*
	 * For each population_entity an economic_unit is defined, and since for each building
	 * a population_entity is defined, then for each building is possible to calculate the average
	 * salary of the person who are living there.
	 */
	class economic_unit
	{
		static long eu_next_unique_id;
		long eu_id;
		long* family_size; //Well, this may be a very large number, depending on the size of the population_entity which encapsulate this eu
		job_market::job_entity* job; //Have this economic unit a job? here are the job information
		game_wallet wallet; //The amount of cash holded by this eu
		salary_class salary_cl;//Class for this salary
		expense_and_cost applicable_costs; //Costs which belongs to this unit.
		revenues_and_profits applicable_revenues;
		/*
		 * The next value are used in the calculation of the perceived equity,
		 * higher is this value then higher is the impact of those factos
		 */
		double net_salary_impact_on_equity,
		       savings_impact_on_equity;
		/*
		 * By default 'false', set to true when the unit is not able to effort all the cost.
		 * When an unit enter this state, may be evicted from his home.
		 */
		bool starving_unit;
		bool have_payed_rental_price;
		double equity_perception_threshold;
		currency_type unit_net_revenue;
	public:
		void set_family_size( long* size )
		{	family_size = size;	}
		void set_costs( const expense_and_cost& );
		void set_revenues( const revenues_and_profits& );
		currency_type get_unit_net_revenue();
		long get_family_size()
		{	return *family_size; }
		bool is_starving()
		{	return starving_unit;	}
		currency_type get_gross_salary();
		bool is_paying_the_rent_price()
		{	return have_payed_rental_price;	}
		economic_unit();
		long get_id();
		job_entity* get_job()
		{	return job;	}
		void set_job( job_entity* n_job )
		{	job = n_job;	}
	public:
		bool look_for_a_job(); // Search and apply for a job
		bool is_unemployed();
		friend class economy_manager;
	};


	class economy_manager : public job_market_manager
	{
		std::mutex eu_container_mutex;
		std::string		player_name;
		vector< economic_unit* > eu_container;
		game_wallet		player_wallet;
		economic_variables*	eco_variables;
		void modify_tax_equity_perception( long double value );
		public_welfare* public_welfare_funds;
	private:
		salary_class calculate_salary_class( currency_type salary );
		currency_type apply_expense_and_cost( economic_unit* eu );
		void calculate_tax_equity_perception( currency_type gross_salary, currency_type net_salary, currency_type savings , economic_unit* eu );
		currency_type get_revenue_from_eu( economic_unit* eu );
		currency_type calculate_gross_salary( economic_unit* eu );
		currency_type calculate_taxes_on_salary( economic_unit* eu );
		currency_type handle_starving_unit( economic_unit* eu , currency_type needed_money );
		void correct_savings_and_salary_impact_on_equity( economic_unit* eu, currency_type net_salary, currency_type savings );
		void calculate_impact_on_equity_factors( economic_unit* eu );
		currency_type collect_money();
		long update_uneployment_level();
	public:
		economy_manager( const std::string& player );
		~economy_manager();
		economic_unit* create_economic_unit( job_entity* job );
		bool add_economic_unit( economic_unit* eu );
		currency_type get_basic_class_salary( salary_class sclass );
		void review_economy();
		const economic_variables* get_the_economic_variables();
		currency_type player_available_money();
		currency_type player_apply_cost( currency_type value );
		currency_type player_add_cash( currency_type value );
		public_welfare* get_public_walfare_funds();
		void remove_starving_state( economic_unit* eu );
		void recalculate_amount_of_starving_unit();
		double set_salary_tax_level( salary_class sc , double new_level );
	};
}

#endif
