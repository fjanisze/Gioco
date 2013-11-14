#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "..\config.hpp"
#include "ui_console.hpp"

#if COMPILE_TEST_MODE

UiConsoleTests::UiConsoleTests()
{
}

UiConsoleTests::~UiConsoleTests()
{
}

void UiConsoleTests::SetUp()
{
}

void UiConsoleTests::TearDown()
{

}

TEST_F( UiConsoleTests , BasicConsoleOperations )
{
    LOG_WARN("STARTING: UiConsoleTests , BasicConsoleOperations");
    //Create a default window configuration
    graphic_ui::game_window_config_t window_config;

    //Create the consoles
    std::vector< graphic_ui::console_wnd_t* > consoles = console.init_consoles( window_config );
    ASSERT_EQ( consoles.size() , 3 ); //Three console are available

    //Add city common buttons
    console.show_city_main_menu();

    //Add city construction buttons
    short amount_of_buttons = console.add_building_construction_btn( *consoles[ 0 ] , 0 ); //consoles[ 0 ] is the 'main console'
    ASSERT_NE( amount_of_buttons , 0 );

    //Now move back to the map manu 'remove all the buttons'
    console.show_map_main_menu();
}

#endif
