#if COMPILE_TEST_MODE

#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "../config.hpp"
#include "../logging/logger.hpp"
#include "gtest.h"

#include "../events.hpp"

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

/*
//Test the events
class my_obj : public events::event_entity
{
    events::event_manager event_mng;
public:
    my_obj()
    {
        long id1 = event_mng.register_event( this );
        long id2 = event_mng.register_event( this );

        events::timer_type timer1 , timer2;

        timer1 = 2;
        timer2 = 5;

        event_mng.start_event_countdown( id1 , &timer1 , events::event_execution::execute_repeat );
        event_mng.start_event_countdown( id2 , &timer2 , events::event_execution::execute_repeat );

        event_mng.main_loop();
    }
    int trigger_event( long id )
    {
        LOG("trigger_event(): ID:", id);
    }
};

void timer_test(void*)
{
    my_obj obj;
}

int main()
{

    _beginthread( &timer_test, 0 , nullptr );
    //Wait for some time
    Sleep( 20000 );
}*/

#endif
