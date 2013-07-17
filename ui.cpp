#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "ui.hpp"

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

    game_map::map_viewport_settings_t game_ui::get_viewport_settings()
    {
        return ui_config.viewport_setting;
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

		//font
		load_and_set_font();

		//Init the console
		init_consoles( ui_config );

		//Default view
		current_view = type_of_view_t::game_map_view;
    }

    game_ui::~game_ui()
    {
    }

    //Set the proper font.
    void game_ui::load_and_set_font()
    {
        if( !ui_config.font.loadFromFile("consola.ttf") )
        {
            LOG_WARN("game_ui::load_and_set_font(): Unable to load the selected font arial.ttf");
        }
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
        LOG("game_ui::main_loop(): Quitting..");
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
        case sf::Event::MouseMoved:
            //The mouse is moving over the game window.
            mouse_moving_event( event );
            break;
        case sf::Event::MouseButtonPressed:
            mouse_press_event( event );
            break;
        default:
            break;
        };
    }

    //The user press one mouse button
    void game_ui::mouse_press_event( const sf::Event& event )
    {
        //Check if the mouse is moving in the gameplay area or over some menu ecc..
        if( is_over_the_game_map( event ) )
        {
            if( current_view == type_of_view_t::game_map_view )
            {
                game_map::field_graphics_t* field = map->get_field_at_pos( event.mouseButton.x , event.mouseButton.y );
                if( field == nullptr )
                {
                    write_info("No action available!");
                    return;
                }
                //the user press the button over the game map
                //The only action allowed is to open the city map if any thus check if there's any city under this position.
                citymap::city_agent* agent = field->manager->get_city_agent();
                if( agent )
                {
                    //Ok.. Entering the city view.
                    LOG("game_ui::mouse_press_event(): Entering the city id:",agent->get_city_id());
                    current_city = agent;
                    current_view = type_of_view_t::city_map_view;
                }
            }
            else if( current_view == type_of_view_t::city_map_view )
            {
                //The user press the button over the city map
            }
        }
    }

    //Manage the mouse moving event
    void game_ui::mouse_moving_event( const sf::Event& event )
    {
        //Check if the mouse is moving in the gameplay area or over some menu ecc..
        if( is_over_the_game_map( event ) )
        {
            //The proper action depends on the type of view.
            if( current_view == type_of_view_t::game_map_view )
            {
                //The mouse is moving over the game map
                game_map_mouse_move( event );
            }
            else if( current_view == type_of_view_t::city_map_view )
            {
                //The mouse is moving over the city map.
                city_map_mouse_move( event );
            }
        }
    }

    void game_ui::game_map_mouse_move( const sf::Event& event )
    {
        static std::stringstream message;
        game_map::field_graphics_t* field = map->get_field_at_pos( event.mouseMove.x , event.mouseMove.y );

        if( field != nullptr )
        {
            //Chage the message
            message.str("");//Clear
            message << "Field name: "<<field->descriptor->name <<", ID: "<<field->field->field_id<<"\nMouse coord: "<<event.mouseMove.x<<","<<event.mouseMove.y<<std::endl;
            write_info( message.str() );
        }
        else
        {
            write_info( "Unknow position" );
        }

        message.str("");
    }

    using namespace std;

    void game_ui::city_map_mouse_move( const sf::Event& event )
    {
        static std::stringstream message;
        //current_city should not be nullptr.
        if( current_city != nullptr )
        {
            //Check for the field and print the information
            citymap::citymap_field_t* field = current_city->get_field_at_pos( event.mouseMove.x , event.mouseMove.y );
            if( field != nullptr )
            {
                message << "Field name: "<<field->descriptor->name <<", ID: "<<field->field_id<<"\nMouse coord: "<<event.mouseMove.x<<","<<event.mouseMove.y<<std::endl;
                write_info( message.str() );
            }
            else
            {
                LOG_ERR("game_ui::city_map_mouse_move(): Moving over an unknow field, unable to find the citymap_field_t");
                write_info("Unable to identify the field..");
            }
        }
        else
        {
            LOG_ERR("game_ui::city_map_mouse_move(): current_city is nullptr, this shouldn't happen");
        }
        message.str("");
    }

    //This function returns true if the user is moving over the playable area
    bool game_ui::is_over_the_game_map( const sf::Event& event )
    {
        if( event.mouseMove.x > ui_config.viewport_setting.map_width  ||
            event.mouseMove.y > ui_config.viewport_setting.map_height )
        {
            return false;
        }
        return true;
    }

    //Redraw the graphic elements.
    void game_ui::screen_refresh()
    {
        window.clear( sf::Color::Black );

        if( current_view == type_of_view_t::game_map_view )
        {
            draw_gameplay_map();
        }
        else if( current_view == type_of_view_t::city_map_view )
        {
            draw_current_city();
        }
        draw_console( window );

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

    //Draw the current city
    void game_ui::draw_current_city()
    {
        std::vector< sf::VertexArray* >* vertex = current_city->get_vertex();
        for( auto &elem : *vertex )
        {
            window.draw( *elem );
        }
    }
}






























