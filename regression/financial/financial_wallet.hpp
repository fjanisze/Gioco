#include "game.hpp"

namespace regression_data
{
	using namespace finance;
	static currency_type input_A_test1( 200 , 50 );
	static currency_type input_B_test1( 0 , 50 );
	static currency_type output_test1( 201 , 0 );

	static currency_type input_A_test2( 325 , 0 );
	static currency_type input_B_test2( 100 , 11 );
	static currency_type output_test2( 224 , 89 );
}

bool currency_sum_operation_test1();
bool currency_sbtract_operation_test2();
bool wallet_subtract_operation_test1();
