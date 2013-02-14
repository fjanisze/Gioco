#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "../common_structures.hpp"
#include "../logging/logger.hpp"
#include "finance.hpp"
#include <string>
#include <queue>
#include <vector>
using namespace std;

#define BASE_AMOUNT_OF_CC 1000
#define CENTRAL_BANK_DEFAULT_RESERVE 10000000000


namespace banking
{
	using namespace finance;

	typedef long bank_account_id;
	class bank_entity;

	/*
	 * This structure describe the bank account information
	 */
	class bank_account
	{
		long owner_id; //The id of the eu which own this bank account

		bank_account_id bank_acc_id; //Should be unique
		currency_type cash_balance; //Cash saldo

		currency_type total_liabilities; //Amount of money borrowed from the bank
		double liabilities_interest_rate; //How expensive are the liabilities?
	public:
		bank_account();
		currency_type withdraw( currency_type amount );
		currency_type deposit( currency_type amount );
		currency_type get_balance()
		{	return cash_balance;	}
		friend class bank_entity;
	};
	
	/*
	 * This class describe a bank entity
	 */
	class bank_entity
	{
		static long next_bank_id;
		long bank_id; //Unique bank id
		string name; //The name of the bank

		currency_type available_cash; //Amount of money available for this bank
		double base_interest_rate; //For the borrowed money

		//Information about the accounts open in this bank_entity
		queue< bank_account_id > next_free_acc_id;
		vector< bank_account* > bank_accounts; //Use the values from next_free_acc_id as indexes
	private:
		bank_account* create_new_account( long owner );
		void set_the_interest_rate( bank_account* acc );
		bool is_existing_bank_acc( bank_account_id acc_id );
	public:
		currency_type bank_reserve_deposit( currency_type amount );
		currency_type bank_reserve_status();
	public:
		bank_account* get_the_account( bank_account_id acc_id );
		bank_entity();
		bank_account* open_new_account( long owner );
		bool closed_bank_account( bank_account_id acc_id );
		//Basic banking operations
		currency_type withdraw( bank_account* acc, currency_type amount );
		currency_type deposit( bank_account* acc, currency_type amount );
		currency_type get_balance( bank_account_id acc_id );
		currency_type get_balance( bank_account* acc );
	};

	/*
	 * This is the class responsible for the banking management.
	 */
	class banking_manager
	{
		bank_entity central_bank; 
	public:
		banking_manager();
		bank_entity* find_bank();
	};
}
