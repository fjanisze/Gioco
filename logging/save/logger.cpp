#include "logger.hpp"
#include "log.hpp"

static logging::logger< logging::file_log_policy >* log_inst;

void create_logger_instance()
{
	log_inst = new logging::logger< logging::file_log_policy >( &logging::logger_inst, "execution.log" ); 
}

void destroy_logger_instance()
{
	LOG("destroy_logger_hinstance(): Destroying log_inst");
	if( log_inst )
		delete log_inst;
}

logging::logger< logging::file_log_policy >* get_inst()
{
	return log_inst;
}

