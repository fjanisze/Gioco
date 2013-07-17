#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "ui_console.hpp"

namespace graphic_ui
{

    ////////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for game_window_config_t and the console manager
	//
	//
	////////////////////////////////////////////////////////////////////

    game_window_config_t::game_window_config_t()
    {
        //Default setting
        full_screen = false;
        //Window size in pixels.
        window_height = 630;
        window_width = 800;
        //Game viewport
        viewport_setting.map_width = 600;
        viewport_setting.map_height = 600;
    }

    console_wnd_t::console_wnd_t( long x_off , long y_off , long wnd_width, long wnd_height )
    {
       create( x_off, y_off , wnd_width , wnd_height );
    }

    console_wnd_t::console_wnd_t()
    {
    }

    void console_wnd_t::create( long x_off , long y_off , long wnd_width, long wnd_height )
    {
        LOG("console_wnd_t::create(): Creating a new console object: ",x_off,",",y_off,",",wnd_width,",",wnd_height);
        x_offset = x_off;
        y_offset = y_off;
        width = wnd_width;
        height = wnd_height;
        //set as quad
        vertex = sf::VertexArray( sf::Quads , 4 );
        //Create the vertex position
        vertex[ 0 ].position = sf::Vector2f( x_off, y_off );
        vertex[ 1 ].position = sf::Vector2f( x_off + width , y_off );
        vertex[ 2 ].position = sf::Vector2f( x_off + width , y_off + height );
        vertex[ 3 ].position = sf::Vector2f( x_off , y_off + height );
        //Set a default color
        set_color( sf::Color( 10 , 30 , 150 ) );
        //Configure properly the text entity
        text.setPosition( vertex[ 0 ].position );
        text.setCharacterSize( 12 );
    }

    void console_wnd_t::set_color( sf::Color color )
    {
        for( short i = 0 ; i < 4 ; i++  )
        {
            vertex[ i ].color = color;
        }
    }

    console_manager::console_manager( )
    {
    }

    //Initializate the console window
    short console_manager::init_consoles( const game_window_config_t& window_config )
    {
        //The initialization is done on the base of the window information.
        long console_height  = window_config.window_height - window_config.viewport_setting.map_height ,
            console_width = window_config.window_width;
        //Info Console
        info_console.create( 0 , window_config.viewport_setting.map_height , console_width , console_height );
        info_console.set_color( sf::Color( 10 , 20 , 30 ) );
        //Main console
        console_height = window_config.window_height - console_height;
        console_width = window_config.window_width - window_config.viewport_setting.map_width;
        main_console.create( window_config.viewport_setting.map_width , 0 , console_width , console_height );
        main_console.set_color( sf::Color( 20 , 30 , 40 ) );

        //Copy the font
        font = &window_config.font;
        main_console.text.setFont( * font );
        info_console.text.setFont( * font );

    }

    //Draw the console in the proper context
    void console_manager::draw_console( sf::RenderWindow& window )
    {
        window.draw( main_console.vertex );
        window.draw( info_console.vertex );
        //text
        window.draw( main_console.text );
        window.draw( info_console.text );
    }

    //Write a message in the info console
    void console_manager::write_info( const std::string& msg )
    {
        info_console.text.setString( msg );
    }

}
