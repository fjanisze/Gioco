#include "log.hpp"

namespace logging
{
	void file_log_policy::open_ostream(const std::string& name)
	{
		out_stream->open( name.c_str(), std::ios_base::binary|std::ios_base::out );
		if( !out_stream->is_open() ) 
		{
			throw(std::runtime_error("LOGGER: Unable to open an output stream"));
		}
	}

	void file_log_policy::close_ostream()
	{
		if(out_stream)
		{
			out_stream->close();
		}
	}

	void file_log_policy::write(const std::string& msg)
	{
		(*out_stream)<<msg<<std::endl;
	}

	std::string file_log_policy::read()
	{
		//Reading not supported
		return std::move( std::string( "" ) );
	}

	file_log_policy::~file_log_policy()
	{
		if( out_stream )
		{
			close_ostream();
		}
	}

	void buffer_log_policy::open_ostream(const std::string& name)
	{
		//Nothing to do
	}

	void buffer_log_policy::close_ostream()
	{
		//Nothing to do
	}

	void buffer_log_policy::write(const std::string& msg)
	{
		buffer.push( msg + "BUFFERED" );
	}

	std::string buffer_log_policy::read()
	{
		//Return the first buffered line
		std::string tmp_str;
		if( !buffer.empty() )
		{
			tmp_str = buffer.front();
			buffer.pop();
		}
		return std::move( tmp_str );
	}
}

