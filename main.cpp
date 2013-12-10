#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "config.hpp"
#include "logging/logger.hpp"
#include "game.hpp"

#include "buildings.hpp"
#include "ui/ui.hpp"
#include "map/city_map.hpp"

#include "ui/draw.hpp"
using namespace drawing_objects;

#if !COMPILE_TEST_MODE

void killer()
{
    std::this_thread::sleep_for( std::chrono::seconds{ 10 } );
    std::terminate();
}

int main()
{
    log_inst.set_thread_name( "MAIN" );

 // std::thread kill( killer );

    drawable_object text_container, vertex_container;

    drawing_facility draw;
    draw.start();

    std::this_thread::sleep_for( std::chrono::milliseconds{ 500 } );

    std::vector< sf::Text* > data;
    sf::Text *t1 = new sf::Text(), *t2 = new sf::Text();
    sf::Text *t3 = new sf::Text();
    data.push_back( t1 );
    data.push_back( t2 );
    data.push_back( t3 );

    std::vector< sf::VertexArray* > data2;
    sf::VertexArray *t4 = new sf::VertexArray( sf::Quads, 4 ), *t5 = new sf::VertexArray( sf::Quads, 4 );
    sf::VertexArray *t6 = new sf::VertexArray( sf::Quads, 4 );
    data2.push_back( t4 );
    data2.push_back( t5 );
    data2.push_back( t6 );

    text_container.update( data , use_font::yes );
    vertex_container.update( data2 );

    //Update vertex position
    vertex_container.get< sf::VertexArray >( 0 )[ 0 ].position = sf::Vector2f( 0 , 0 );
    vertex_container.get< sf::VertexArray >( 0 )[ 1 ].position = sf::Vector2f( 100 , 0 );
    vertex_container.get< sf::VertexArray >( 0 )[ 2 ].position = sf::Vector2f( 100 , 100 );
    vertex_container.get< sf::VertexArray >( 0 )[ 3 ].position = sf::Vector2f( 0 , 100 );
    vertex_container.get< sf::VertexArray >( 0 )[ 0 ].color = sf::Color::Cyan;
    vertex_container.get< sf::VertexArray >( 0 )[ 1 ].color = sf::Color::Cyan;
    vertex_container.get< sf::VertexArray >( 0 )[ 2 ].color = sf::Color::Cyan;
    vertex_container.get< sf::VertexArray >( 0 )[ 3 ].color = sf::Color::Cyan;

    vertex_container.get< sf::VertexArray >( 1 )[ 0 ].position = sf::Vector2f( 300 , 300 );
    vertex_container.get< sf::VertexArray >( 1 )[ 1 ].position = sf::Vector2f( 400 , 300 );
    vertex_container.get< sf::VertexArray >( 1 )[ 2 ].position = sf::Vector2f( 400 , 400 );
    vertex_container.get< sf::VertexArray >( 1 )[ 3 ].position = sf::Vector2f( 300 , 400 );
    vertex_container.get< sf::VertexArray >( 1 )[ 0 ].color = sf::Color::Cyan;
    vertex_container.get< sf::VertexArray >( 1 )[ 1 ].color = sf::Color::Cyan;
    vertex_container.get< sf::VertexArray >( 1 )[ 2 ].color = sf::Color::Cyan;
    vertex_container.get< sf::VertexArray >( 1 )[ 3 ].color = sf::Color::Cyan;

    text_container.get< sf::Text >( 0 ).setPosition( sf::Vector2f( 400, 400 ) );
    text_container.get< sf::Text >( 0 ).setCharacterSize( 30 );
    text_container.get< sf::Text >( 0 ).setColor( sf::Color::White );
    text_container.get< sf::Text >( 0 ).setStyle( sf::Text::Regular );
    text_container.get< sf::Text >( 0 ).setString("MERDAAA");


    draw.add( &vertex_container );
    draw.add( &text_container );

    std::this_thread::sleep_for( std::chrono::milliseconds{ 2000 } );

    draw.terminate();/*

    game_manager::game_manager* game_manager = game_manager::game_manager::get_instance();
    game_manager->init();

	//Creating the game scenario
	game_manager->create_test_scenario_1();
    game_manager->handle_game();


    log_inst.terminate_logger();*/
//	delete game_manager::game_manager::get_instance();
	return 0;
}

#endif
