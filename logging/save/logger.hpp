#include "log.hpp"

#ifndef LOGGER_HPP
#define LOGGER_HPP

void create_logger_instance();
void destroy_logger_instance();

logging::logger< logging::file_log_policy >* get_inst();


#endif


#ifdef LOGGING_LEVEL_1
#define LOG get_inst()->print
#define LOG_ERR get_inst()->print_error
#define LOG_WARN get_inst()->print_warning
#else
#define LOG(...) 
#define LOG_ERR(...)
#define LOG_WARN(...)
#endif

#ifdef LOGGING_LEVEL_2
#define ELOG get_inst()->print
#define ELOG_ERR get_inst()->print_error
#define ELOG_WARN get_inst()->print_warning
#else
#define ELOG(...) 
#define ELOG_ERR(...)
#define ELOG_WARN(...)
#endif
