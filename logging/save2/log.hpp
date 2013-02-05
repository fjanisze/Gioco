#include <iostream>
#include <string>
#include <queue>
#include <fstream>
#include <stdexcept>
#include <memory>
#include <ctime>
#include <sstream>
#include <mutex>
#include <vector>

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
		virtual void		open_ostream(const std::string& name) = 0;
		virtual void		close_ostream() = 0;
		virtual void		write(const std::string& msg) = 0;
		virtual std::string	read() = 0;

	};

	/*
	 * Implementation allowing log to a file
	 */

	class file_log_policy : public log_policy_interface
	{
		std::unique_ptr< std::ofstream > out_stream;
	public:
		file_log_policy() : out_stream( new std::ofstream ) {}
		void open_ostream(const std::string& name);
		void close_ostream();
		void write(const std::string& msg);
		std::string read();
		~file_log_policy();
	};
	
	class buffer_log_policy : public log_policy_interface
	{
		std::queue< std::string > buffer;
	public:
		void open_ostream(const std::string& name);
		void close_ostream();
		void write(const std::string& msg);
		std::string read();
	};

	/*
	 * Support class
	 */

	template< typename log_policy >
	class logger// : public logger_interface
	{
		unsigned log_line_number;
		std::string get_time();
		std::string get_logline_header();
		std::stringstream log_stream;
		log_policy* policy;
		//Core printing functionality
		void print_impl();
		template<typename First, typename...Rest>
		void print_impl(First parm1, Rest...parm);
	public:
		logger( const std::string& filename );
		template< typename...Args >
		void print(Args...parms);
		template< typename...Args >
		void print_error(Args...parms);
		template< typename...Args >
		void print_warning(Args...parms);

		template< typename source_log_type >
		void operator<<( source_log_type& );

		~logger();
		log_policy* get_policy();
	};

	/*
	 * Implementation for logger
	 */

	template< typename log_policy >
	logger< log_policy >::logger( const std::string& filename )
	{
		log_line_number = 0;
		policy = new log_policy;
		if( !policy )
		{
			throw std::runtime_error("LOGGER: Unable to create the logger instance"); 
		}
		policy->open_ostream( filename );
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

		header.str("");
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
		policy->write( get_logline_header() + log_stream.str() );
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
		if( policy )
		{
			policy->close_ostream();
		}
	}

	template< typename log_policy >
		template< typename source_log_type >
	void logger< log_policy >::operator<< ( source_log_type& source )
	{
		std::string tmp_str;
		do{
			tmp_str = source->get_policy()->read();
			if( tmp_str.empty() ) break;
			policy->write( tmp_str );
		}while( 1 );
	}

	template< typename log_policy >
	log_policy* logger< log_policy >::get_policy()
	{
		return policy;
	}
}

#endif
