#include "finance.hpp"

namespace finance
{
	game_wallet::game_wallet() : money_cash( 0 )
	{	}

	game_wallet& game_wallet::operator=( const game_wallet& wl )
	{
		money_cash = wl.money_cash;
		return *this;
	}

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

	currency_type& game_wallet::get_money_cash()
	{
		return money_cash;
	}

}
