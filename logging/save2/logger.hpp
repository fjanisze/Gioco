#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "log.hpp"
#include <mutex>

static logging::logger< logging::file_log_policy > log_inst( "execution.log" );

enum severity_type
{
	normal = 1,
	error,
	warning
};


template< short severity, typename... Args >
void get_inst( Args... args )
{

	static std::mutex write_mutex;
	static __thread logging::logger< logging::buffer_log_policy >* buffer = nullptr; 
	if( !write_mutex.try_lock() )
	{
		//Cannot write right now, buffering..
		if( buffer == nullptr )
		{
			buffer = new logging::logger< logging::buffer_log_policy >("dummy");
		}
		switch( severity )
		{
		case 2:
			buffer->print_error( args... );
			break;
		case 3:
			buffer->print_warning( args... );
			break;
		case 1:
			buffer->print( args... );
			break;
		};
	}
	else
	{
		//Are there buffered messages?
		if( buffer != nullptr )
		{
			//Print the buffered line
			log_inst << buffer;
			delete buffer;
			buffer = nullptr;
		}
		
		switch( severity )
		{
		case 2:
			log_inst.print_error( args... );
			break;
		case 3:
			log_inst.print_warning( args... );
			break;
		case 1:
			log_inst.print( args... );
			break;
		};
		write_mutex.unlock();
	}
}


#ifdef LOGGING_LEVEL_1
#define LOG get_inst< 1 >
#define LOG_ERR get_inst < 2 >
#define LOG_WARN get_inst< 3 >
#else
#define LOG(...) 
#define LOG_ERR(...)
#define LOG_WARN(...)
#endif

#ifdef LOGGING_LEVEL_2
#define ELOG get_inst< 1 >
#define ELOG_ERR get_inst< 2 >
#define ELOG_WARN get_inst< 3 >
#else
#define ELOG(...) 
#define ELOG_ERR(...)
#define ELOG_WARN(...)
#endif

#endif
