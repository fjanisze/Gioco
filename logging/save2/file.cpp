#include "file.hpp"

void thread_a(void*)
{
	for( long i = 0 ; i < 1000 ; i++ )
	{
		LOG("void thread_a(): Logging #", i );
		Sleep(10);
	}

}

void thread_b(void*)
{
	for( long i = 0 ; i < 1000 ; i++ )
	{
		LOG("void thread_b(): Logging #", i );
		Sleep(5);
	}

}

void thread_c(void*)
{

}
