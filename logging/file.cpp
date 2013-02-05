#include "file.hpp"

void thread_a(void*)
{
	for( long i = 0 ; i < 1000 ; i++ )
	{
//		Sleep(4);
		LOG_ERR("void thread_a(): Logging #", i );
	}

}

void thread_b(void*)
{
	for( long i = 0 ; i < 1000 ; i++ )
	{
	//	Sleep(5);
		LOG("void thread_b(): Logging #", i );
	}

}

void thread_c(void*)
{
	for( long i = 0 ; i < 1000 ; i++ )
	{
		LOG_WARN("int thread_c(): Logging #", i );
//		Sleep(2);
	}
}
