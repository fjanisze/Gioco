#include "file.hpp"

int main()
try{
//	_beginthread( thread_a, 0 , nullptr );
//	_beginthread( thread_b, 0 , nullptr );

	for( long i = 0 ; i < 1000 ; i++ )
	{
		LOG_WARN("int main(): Logging #", i );
		Sleep(2);
	}

	Sleep(5000);

}catch( std::exception& xa )
{
	std::cerr<<xa.what()<<std::endl;
	return 0;
}catch( ... )
{
	std::cerr<<"Some exeption\n";
	return 0;
}
