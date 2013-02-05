#include <iostream>
#include <string>
#include <windows.h>
#include "financial/financial_wallet.hpp"

typedef bool (*test_pointer)();

struct test_macro
{
	test_pointer function;
	string name;
};

test_macro test_table[] =
{
	{ &currency_sum_operation_test1, "currency_sum_operation_test1" },
	{ &currency_sbtract_operation_test2, "currency_sbtract_operation_test2" },
	{ &wallet_subtract_operation_test1, "wallet_subtract_operation_test1" },
	{ nullptr , "dummy" }
};

struct colors 
{
	long color;
	colors( long possible_color ) : color( possible_color )
	{	}
};

static colors color_fore_white( FOREGROUND_WHITE );
static colors color_fore_red( FOREGROUND_RED );
static colors color_fore_iwhite( WHITE_INTENSE_COLOR );
static colors color_fore_green( FOREGROUND_GREEN );
static colors color_fore_yellow( FOREGROUND_GREEN | FOREGROUND_RED );

std::ostream& operator<<( ostream& out_stream , const colors& color )
{
	SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), color.color );
	return out_stream;
}

int main()
{
	using namespace std;
	cout<<color_fore_yellow<<"Running the regression.."<<color_fore_white<<endl<<endl;
	for( int i = 0 ; test_table[ i ].function != nullptr ; i++ )
	{
		cout<<"#"<<i + 1<<" - "<<"running "<<color_fore_yellow<<test_table[ i ].name<<color_fore_white<<" --> Result: ";
		if( test_table[ i ].function() )
		{
			cout<<color_fore_green<<"PASS"<<color_fore_white<<endl;
		}
		else
		{
			cout<<color_fore_red<<"FAIL"<<color_fore_white<<endl;
		}
	}
}
