#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#define GAME_SPEED 20

#ifndef EVENTS_HPP
#define EVENTS_HPP

#include "logging/logger.hpp"
#include <ctime>
#include <mutex>
#include <list>


namespace events
{
	class event_entity
	{
	public:
		virtual int trigger_event( long event_id ) = 0;
	};

	typedef enum event_execution
	{
		execute_oneshot,
		execute_repeat,
	} event_execution;

	class event_manager;

	typedef struct timer_type
	{
	private:
		long timer_value;
	public:
		timer_type& operator=( const long value );
		bool operator>( const long value );
		bool operator<( const long value );
		long get_time_left() const;
		friend class event_manager;
	} timer_type;

	typedef struct event_obj
	{
		event_entity* event_pointer;
		long event_id;
		timer_type* timer;
		timer_type timer_copy;
		event_execution execution_rule;
		event_obj() : event_id( 0 ) , timer( nullptr ) , event_pointer( nullptr )
		{}
	} event_obj;

	class event_manager
	{
		static event_manager* instance;
		std::mutex event_mutex;
		std::list< event_obj > events;
		long next_event_id;
		event_obj* get_event_obj( long event_id );
		bool decrease_and_check( event_obj* );
	public:
		static event_manager* get_instance();
		event_manager();
		long register_event( event_entity* target_event ); 
		bool unregister_event( long event_id );
		bool start_event_countdown( long event_id, timer_type* timer ,event_execution exec_type ); 
	public:
		void main_loop(); 
	};
}

#endif
