#include "events.hpp"

namespace events
{
	timer_type& timer_type::operator=( const long value )
	{
		timer_value = value;
		return *this;
	}

	bool timer_type::operator>( const long value )
	{
		return timer_value > value;
	}

	bool timer_type::operator<( const long value )
	{
		return timer_value < value;
	}

	long timer_type::get_time_left() const
	{
		return timer_value;
	}

	event_manager* event_manager::instance = nullptr;

	event_manager* event_manager::get_instance()
	{
		if( instance == nullptr )
		{
			instance = new event_manager; //TODO: This memory has to be freed
		}
		return instance;
	}

	event_manager::event_manager() : next_event_id( 1 )
	{
		LOG("event_manager::event_manager(): Creating a new event_manager");
	}

	long event_manager::register_event( event_entity* target_event )
	{
		ELOG("event_manager::register_event(): Registering new event ");
		if( target_event == nullptr )
		{
			LOG_ERR("event_manager::register_event(): target_event is null, an exception will be thrown ");
			throw std::runtime_error("event_manager::register_event(): target_event cannot be null!");
		}
		std::lock_guard< std::mutex > lock( event_mutex );
		long event_id = next_event_id++;
		event_obj event;
		event.event_pointer = target_event;
		event.event_id = event_id;
		events.push_back( event );
		return event_id;
	};

	event_obj* event_manager::get_event_obj( long event_id )
	{
		for( auto elem = begin( events ) ; elem != end( events ) ; elem++ )
		{
			if( elem->event_id == event_id )
			{
				return &(*elem);
			}
		}
		return nullptr;
	}

	bool event_manager::start_event_countdown( long event_id, timer_type* timer , event_execution exec_type )
	{
		ELOG("event_manager::start_event_countdown(): Starting countdown for event_id ", event_id , " timer ", timer->timer_value, " sec" );
		std::lock_guard< std::mutex > lock( event_mutex );
		event_obj* obj = get_event_obj( event_id );
		if( obj )
		{
			obj->timer = timer;
			obj->execution_rule = exec_type;
			obj->timer_copy = *timer;
			return true;
		}
		LOG_ERR("event_manager::start_event_countdown(): Unable to find the event id :" , event_id);
		return false;
	}

	bool event_manager::decrease_and_check( event_obj* event )
	{
		if( event->timer->timer_value > 0  && --event->timer->timer_value == 0 )
			return true;
		return false;
	}

	void event_manager::main_loop()
	{
		LOG("event_manager::main_loop(): Starting");
		do{
			event_mutex.lock();
			//Loop through all the events
			for( auto elem : events )
			{
				if( decrease_and_check( &elem ) ) //Is the timer expired?
				{
					//The timer is expired, let's trigger the action
					ELOG("event_manager::main_loop(): Triggering event ", elem.event_id );
					elem.event_pointer->trigger_event( elem.event_id );
					//Apply te execution rule
					if( elem.execution_rule == event_execution::execute_repeat )
					{
						ELOG("event_manager::main_loop(): Restarting the cyclic event: ", elem.event_id );
						//This is a ripetitive event, restart the timer
						*elem.timer = elem.timer_copy;
					}
					else
					{
						//No more repetition, the element will be marked and then removed
						ELOG("event_manager::main_loop(): Signing the expired event: ", elem.event_id);
						elem.event_id = -1; //invalid Id, used as marker
					}
				}
			}
			//Removing the marked elements
			for( std::list< event_obj >::iterator it = begin( events ) ; it != end( events ) ; it++ )
			{
				if( it->event_id < 0 )
				{
					events.erase( it );
				}
			}
			event_mutex.unlock();
			std::this_thread::sleep_for( std::chrono::seconds{ 1 } );
		}while( 1 );
		LOG("event_manager::main_loop(): Quitting");
	}
}

