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
        };
    }

    //Redraw the convas
    void game_ui::screen_refresh()
    {
        window.clear();
        window.display();
    }
}






























