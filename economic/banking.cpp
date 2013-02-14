#include "banking.hpp"

namespace banking
{

	bank_account::bank_account() : owner_id ( -1 ), 
			bank_acc_id( -1 ), cash_balance( 0 ),
			total_liabilities( 0 ), liabilities_interest_rate( 0 )
	{
	}

	currency_type bank_account::withdraw( currency_type amount )
	{
		if( cash_balance >= amount )
		{
			cash_balance -= amount;
			return cash_balance;
		}
		return -1;
	}

	currency_type bank_account::deposit( currency_type amount )
	{
		cash_balance += amount;
		return cash_balance;
	}

	long bank_entity::next_bank_id = 1;

	bank_entity::bank_entity()
	{
		bank_id = next_bank_id++;
		LOG("bank_entity::bank_entity(): New bank entity, ID:",bank_id );

		available_cash = 0;
		base_interest_rate = 5.5; //%

		//Create a default ammount of possible bank accounts ( all empty )
		bank_accounts.reserve( BASE_AMOUNT_OF_CC );
		//Create the 'list' of bank account number
		for( int i = 0 ; i < BASE_AMOUNT_OF_CC ; i++ )
		{
			next_free_acc_id.push( i ); 
		}
	}

	//Open a new account
	bank_account* bank_entity::open_new_account( long owner )
	{
		bank_account_id id = -1;
		bank_account* acc = create_new_account( owner );
		if( acc != nullptr )
		{
			id = acc->bank_acc_id;
			set_the_interest_rate( acc );
			//Adding the account
			bank_accounts[ id ] = acc;
			LOG("bank_entity::open_new_account(): New bank account, ID:",id,", owner EU:",owner );
		}
		return acc;
	}

	//Create the new account allocating the memory ecc
	bank_account* bank_entity::create_new_account( long owner )
	try{
		if( next_free_acc_id.empty() )
		{
			LOG_ERR("bank_entity::create_new_account(): Cannot open a new account!");
			return nullptr;
		}

		bank_account_id id = next_free_acc_id.front();
		next_free_acc_id.pop();
	
		bank_account* acc = new bank_account;

		acc->owner_id = owner;
		acc->bank_acc_id = id;

	}catch( std::bad_alloc& xa )
	{
		LOG_ERR("bank_entity::create_new_account: Exception catched, what: ",xa.what() );
		return nullptr;
	}
	
	//Calculate and set the proper interest rate
	void bank_entity::set_the_interest_rate( bank_account* acc )
	{
		acc->liabilities_interest_rate = base_interest_rate;
	}

	bool bank_entity::is_existing_bank_acc( bank_account_id acc_id )
	{
		return ( acc_id < bank_accounts.size() ) && ( bank_accounts[ acc_id ] != nullptr );
	}

	//Close the bank accout 
	bool bank_entity::closed_bank_account( long acc_id )
	{
		bool verdict = false;
		if( is_existing_bank_acc( acc_id) )
		{
			//The account will be closed no matter how many money are in there
			bank_account* acc = bank_accounts[ acc_id ];
			bank_accounts[ acc_id ] = nullptr;
			
			next_free_acc_id.push( acc_id ); //Free again
			ELOG("bank_entity::closed_bank_account(): accID:",acc_id,", closed!");
			verdict = true;
		}
		return verdict;
	}

	//Check if the acc_id is valid and return the poiner to the bank account structure
	bank_account* bank_entity::get_the_account( bank_account_id acc_id )
	{
		if( is_existing_bank_acc( acc_id ) )
		{
			return bank_accounts[ acc_id ];
		}
		return nullptr;
	}
	
	currency_type bank_entity::withdraw( bank_account* acc, currency_type amount )
	{
		currency_type amount_left = -1;
		if( acc != nullptr )
		{
			if( acc->cash_balance >= amount )
			{
				acc->cash_balance -= amount;
				amount_left = acc->cash_balance;
			}
		}
		return amount_left;
	}

	currency_type bank_entity::deposit( bank_account* acc, currency_type amount )
	{
		currency_type amount_left = -1;
		if( acc != nullptr )
		{
			acc->cash_balance += amount;
			amount_left = acc->cash_balance;
		}
		return amount_left;
	}

	currency_type bank_entity::get_balance( bank_account_id acc_id )
	{
		if( is_existing_bank_acc( acc_id ) )
		{
			return bank_accounts[ acc_id ]->cash_balance;
		}
		return -1;
	}

	currency_type bank_entity::get_balance( bank_account* acc )
	{
		return acc->cash_balance;
	}

	//Increase the bank financial reserve
	currency_type bank_entity::bank_reserve_deposit( currency_type amount )
	{
		available_cash += amount;
	}

	currency_type bank_entity::bank_reserve_status()
	{
		return available_cash;
	}

	//////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for banking manager
	//
	//
	//////////////////////////////////////////////////////////////////
	
	banking_manager::banking_manager()
	{
		LOG("banking_manager::banking_manager(): Banking manager created");
		central_bank.bank_reserve_deposit( CENTRAL_BANK_DEFAULT_RESERVE );
	}

	/*
	 * Actually this function does not make any search opeation,
	 * just return a pointer to the only available bank
	 */
	bank_entity* banking_manager::find_bank()
	{
		return &central_bank;
	}
}
