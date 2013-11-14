#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "..\config.hpp"
#include "construction.hpp"
#include "..\construction_common_struct.hpp"

#if COMPILE_TEST_MODE

ConstructionTests::ConstructionTests()
{
    game = game_manager::game_manager::get_instance();
    game->init();
    //Generate the random map.
    game->get_the_game_map()->create_new_map( 50 );
    game->get_the_game_map()->generate_random_map();

    //Create the cities in the game.
    roma_agent = game->get_city_manager()->create_new_city( "Roma", 100 );

    //Create the cities on the map
    game->get_the_game_map()->create_a_city_at_random_coord( roma_agent );
}

ConstructionTests::~ConstructionTests()
{
    game->stop();
}

void ConstructionTests::SetUp()
{
}

void ConstructionTests::TearDown()
{

}

TEST_F( ConstructionTests , TriggerOneBuildingConstruction )
{
    LOG_WARN("STARTING: ConstructionTests , TriggerOneBuildingConstruction");
    //Emulate the click of one button
    city_ui_manager::city_ui* city_ui = game->get_game_ui()->get_city_ui();
    ASSERT_NE( city_ui , nullptr );
    city_ui->set_cityagent( roma_agent );
    city_ui->handle_build_btn_click( 1 );

    //At this point, the user selected the building with id 1, now emulate the click on the map
    //Create the event
    game_map::game_canvas_settings_t game_canvas = graphic_ui::game_ui::get_instance()->get_game_canvas_settings();
    sf::Event event;

    //The point should be on the map
    event.mouseButton.x = game_canvas.canvas_x_offset + 100;
    event.mouseButton.y = game_canvas.canvas_y_offset + 100;

    //Trigger the construction
    bool verdict = city_ui->mouse_press_event( event );
    ASSERT_TRUE( verdict );
}

#endif
