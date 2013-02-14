#ifndef FINANCE_HPP
#define FINANCE_HPP

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
		game_wallet();

		game_wallet( const currency_type& value );
		game_wallet( const game_wallet& wallet );

		game_wallet& operator=( const game_wallet& );
		game_wallet& operator=( const currency_type& value );

		currency_type available_free_cash();
		currency_type& get_money_cash();
	};
}

#endif
