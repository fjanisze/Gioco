#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "ui.hpp"

namespace graphic_ui
{

    ////////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for game_window_config_t
	//
	//
	////////////////////////////////////////////////////////////////////

    game_window_config_t::game_window_config_t()
    {
        //Default setting
        full_screen = false;
        //Window size in pixels.
        window_height = 600;
        window_width = 800;
        //Game viewport
        viewport_setting.map_width = 600;
        viewport_setting.map_height = 600;
    }

    ////////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for game_ui
	//
	//
	////////////////////////////////////////////////////////////////////

    game_ui* game_ui::instance = nullptr;

    game_ui* game_ui::get_instance()
    {
        if( instance == nullptr )
		{
			instance = new game_ui;
		}
		return instance;
    }

    game_ui::game_ui()
    {
        LOG("game_ui::game_ui(): Window width: " , ui_config.window_width, ", height: ", ui_config.window_height, ", full screen: ", ui_config.full_screen, ". Creating the window" );
        is_the_window_running = false;
        //Create the proper instance, check if this is the only one
        assert( instance == nullptr );
        instance = this;
        //Create the viewport
		map = game_map::game_map::get_instance();
		map->configure_viewport( ui_config.viewport_setting );
		//Populate the vertex map
		map->create_vertex_map();
        //Create the window
        create_render_window();
    }

    game_ui::~game_ui()
    {
    }

    //Configure and create the appropriate window
    sf::RenderWindow& game_ui::create_render_window()
    {
        LOG("game_ui::create_render_window(): Creating the rendering window" );
        //Create the resource
        //Video mode:
        sf::VideoMode video_mode;
        video_mode.bitsPerPixel = 24;
        video_mode.height = ui_config.window_height;
        video_mode.width = ui_config.window_width;
        //Context settings:
        sf::ContextSettings context;
        context.depthBits = 24;
        context.stencilBits = 8;
        context.minorVersion = 0;
        context.majorVersion = 3;
        context.antialiasingLevel = 4;
        //Style
        UINT32 style = sf::Style::Default;
        //Full screen?
        if( ui_config.full_screen )
        {
            style |= sf::Style::Fullscreen;
        }

        window.create( video_mode , "Economics" , style , context );
        is_the_window_running = true;

        return window;
    }

    //Main loop handling the graphic interface
    void game_ui::main_loop()
    {
        while( is_the_window_running )
        {
            sf::Event event;
            while( window.pollEvent( event ))
            {
                handle_event( event );
            }

            screen_refresh();
        }
        //We are quitting, let's clean
        map->destroy_vertex_map();
    }

    //Handle the events from the window
    void game_ui::handle_event( const sf::Event& event )
    {
        switch( event.type )
        {
        case sf::Event::Closed:
            is_the_window_running = false;
            LOG("game_ui::handle_event(): Closing the window, was requested to quit");
            break;
        default:
            break;
        };
    }

    //Redraw the graphic elements.
    void game_ui::screen_refresh()
    {
        window.clear( sf::Color::White );

        draw_gameplay_map();

        window.display();
    }

    //Draw the gameplay map
    void game_ui::draw_gameplay_map()
    {
        game_map::field_graphic_vector_t* vertex = map->get_vertex_data();
        for( auto &elem : *vertex )
        {
            window.draw( *elem->vertex );
        }
    }
}






























