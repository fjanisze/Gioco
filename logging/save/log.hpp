#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>
#include <map>
#include <memory>
#include <algorithm>
#include <ctime>
#include <sstream>

#ifndef LOG_HPP
#define LOG_HPP

namespace logging
{

	/* 
	 * Interface for the logger
	 */

	class log_policy_interface
	{
	public:
		virtual void	    open_ostream(const std::string& name) = 0;
		virtual void 	    close_ostream() = 0;
		virtual void 	    write(const std::string& msg) = 0;
	};

	/*
	 * Implementation allowing log to a file
	 */

	class file_log_policy : public log_policy_interface
	{
		std::unique_ptr<std::ofstream> out_stream;
	public:
		file_log_policy() : out_stream( new std::ofstream ) {}
		void open_ostream(const std::string& name);
		void close_ostream();
		void write(const std::string& msg);
		~file_log_policy();
	};


	/*
	 * Handle logging operation
	 */

	template< typename log_policy >
	class log_manager
	{
		static unsigned next_handler_id;
		std::map< unsigned,  log_policy*  > log_handlers;
		typedef typename std::map< unsigned, log_policy*  >::iterator log_handler_it;
		void close_and_erase(log_handler_it& it);
	public:
		log_manager();
		unsigned log_open(const std::string& logname);
		void     log_close(unsigned handler);
		inline void print(unsigned handler, const std::string& msg);
		~log_manager();

	};

	template<typename log_policy>
	unsigned log_manager< log_policy >::next_handler_id = 1;

	/*
	 * 
	 * Implementation for log_manager
	 *
	 */

	template<typename log_policy>
	log_manager< log_policy >::log_manager() : log_handlers()
	{
	}

	template<typename log_policy>
	unsigned log_manager< log_policy >::log_open(const std::string& logname)
	{
		unsigned current_id = next_handler_id++;
		log_policy* policy = new log_policy ;
		if(policy)
		{
			policy->open_ostream(logname);
			log_handlers[current_id] = policy;
		}
		return current_id;
	}

	template<typename log_policy>
	log_manager< log_policy >::~log_manager()
	{
		if( log_handlers.size() )
		{
			std::for_each( begin(log_handlers), end(log_handlers), 
				[](std::pair< unsigned , log_policy* >&& p){ p.second->close_ostream(); });
		}
	}

	template<typename log_policy>
	void log_manager< log_policy >::log_close(unsigned handler)
	{
		auto it = log_handlers.find(handler);
		if ( it != end( log_handlers) )
		{
			close_and_erase(it);
		}
	}

	template< typename log_policy > 
	void log_manager< log_policy >::close_and_erase(log_handler_it& it)
	{
		it->second->close_ostream();
		log_handlers.erase(it);
	}

	template< typename log_policy > inline
	void log_manager< log_policy >::print(unsigned handler, const std::string& msg)
	{
		auto it = log_handlers.find(handler);
		if( it!=end(log_handlers) )
		{
			it->second->write( msg );
		}
	}

	/*
	 * Log manager instance
	 */

	static log_manager< file_log_policy > logger_inst;

	/*
	 * Support class
	 */

	template< typename log_policy >
	class logger// : public logger_interface
	{
		unsigned log_handler;
		unsigned log_line_number;
		std::string get_time();
		std::string get_logline_header();
		log_manager< log_policy >* logger_inst;
		std::stringstream log_stream;
		//Core printing functionality
		void print_impl();
		template<typename First, typename...Rest>
		void print_impl(First parm1, Rest...parm);
	public:
		logger(log_manager< log_policy >* log_h, const std::string& filename);

		template< typename...Args >
		void print(Args...parms);
		template< typename...Args >
		void print_error(Args...parms);
		template< typename...Args >
		void print_warning(Args...parms);

		~logger();
	};

	/*
	 * Implementation for logger
	 */

	template< typename log_policy >
	logger< log_policy >::logger( log_manager< log_policy >* log_h, const std::string& filename )
	{
		logger_inst = log_h;
		log_line_number = 0;
		log_handler = logger_inst->log_open(filename);
	}

	template< typename log_policy >
	std::string logger< log_policy >::get_time()
	{
		std::string time_str;
		time_t raw_time;
		
		time( & raw_time );
		time_str = ctime( &raw_time );

		//without the newline character
		return time_str.substr( 0 , time_str.size() - 2 );
	}

	template< typename log_policy >
	std::string logger< log_policy >::get_logline_header()
	{
		std::stringstream header;

		header.str("");	 //clear the stringstream
		header.fill('0');
		header.width(7);
		header << log_line_number++ <<" < "<<get_time()<<" - ";

		header.fill('0');
		header.width(7);
		header <<clock()<<" > ~ ";

		return header.str();
	}

	template< typename log_policy >
	void logger< log_policy >::print_impl()
	{
		logger_inst->print( log_handler, get_logline_header() + log_stream.str() );
		log_stream.str("");
	}

	template< typename log_policy >
		template<typename First, typename...Rest >
	void logger< log_policy >::print_impl(First parm1, Rest...parm)
	{
		log_stream<<parm1;
		print_impl(parm...);	
	}

	template< typename log_policy >
		template< typename...Args >
	void logger< log_policy >::print(Args...parm)
	{
		log_stream<<"<LOGGER> :";
		print_impl(parm...);
	}


	template< typename log_policy >
		template< typename...Args >
	void logger< log_policy >::print_error(Args...parm)
	{
		log_stream<<"<ERROR> : ";
		print_impl(parm...);
	}

	template< typename log_policy >
		template< typename...Args >
	void logger< log_policy >::print_warning(Args...parm)
	{
		log_stream<<"<WARNING> : ";
		print_impl(parm...);
	}

	template< typename log_policy >
	logger< log_policy >::~logger()
	{
		logger_inst->log_close(log_handler);
	}
}

#endif
