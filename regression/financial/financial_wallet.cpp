#include "financial_wallet.hpp"
#include <iostream>
using namespace std;

using namespace finance;
using namespace regression_data;

bool currency_sum_operation_test1()
{
	currency_type result = input_A_test1 + input_B_test1;
	return ( result.integer_part == output_test1.integer_part ) && ( result.decimal_part == output_test1.decimal_part ); 
}

bool currency_sbtract_operation_test2()
{
	currency_type result = input_A_test2 - input_B_test2;
	bool verdict = ( result.integer_part == output_test2.integer_part ) && ( result.decimal_part == output_test2.decimal_part ); 
	return verdict;
}

bool wallet_subtract_operation_test1()
{
	//First part of the test
	currency_type amount1( 150 , 30 ),
		      amount2( 70 , 50 ),
		      expected_amount( 79 , 80 );
	player_wallet result,
		      wallet1( amount1 ),
		      wallet2( amount2 ),
		      expected( expected_amount );
	result = wallet1 -= wallet2;
	bool verdict = result == expected;
	//Second part of the test
	result -= expected_amount;
	expected_amount = 0;
	return verdict && ( result == expected_amount );
}
