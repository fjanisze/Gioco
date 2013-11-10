#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "ui.hpp"

namespace graphic_ui
{

    //////a//////////////////////////////////////////////////////////////
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


    game_map::game_canvas_settings_t game_ui::get_game_canvas_settings()
    {
        return ui_config.map_canvas_setting;
    }

    game_ui::game_ui()
    {
        LOG("game_ui::game_ui(): Window width: " , ui_config.window_width, ", height: ", ui_config.window_height, ", full screen: ", ui_config.full_screen, ". Creating the window" );
        is_the_window_running = false;
        //font
		load_and_set_font();

        //Create the proper instance, check if this is the only one
        assert( instance == nullptr );
        instance = this;
        //Create the viewport
		map = game_map::game_map::get_instance();
		map->configure_game_canvas( ui_config.map_canvas_setting );

		//Create the city_ui object
		city_ui = new( std::nothrow ) city_ui_manager::city_ui( &window , ui_config.map_canvas_setting );
		assert( city_ui != nullptr );

		//Set the console manager
		city_ui->set_console_manager( this );

		//Init the console
		init_consoles( ui_config );

		//Default view
		current_view = type_of_view_t::game_map_view;
    }

    //Return the pointer to the city_ui
    city_ui_manager::city_ui* game_ui::get_city_ui()
    {
        return city_ui;
    }

    game_ui::~game_ui()
    {
    }

    //Set the proper font.
    void game_ui::load_and_set_font()
    {
        std::lock_guard< std::mutex > lock( mutex );
        ELOG("game_ui::load_and_set_font(): Loading font consola.ttf");
        if( !ui_config.font.loadFromFile("consola.ttf") )
        {
            LOG_WARN("game_ui::load_and_set_font(): Unable to load the selected font consola.ttf");
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
        LOG("game_ui::main_loop(): Entering..");
        while( is_the_window_running )
        {
            sf::Event event;
            while( window.pollEvent( event ))
            {
                //Different handle is called on the base of the type of view
                switch( current_view )
                {
                case type_of_view_t::game_map_view:
                    handle_event( event );
                    break;
                case type_of_view_t::city_map_view:
                    //Call the city_ui object to handle the event
                    if( city_ui->handle_event( event ) != 0 )
                    {
                        is_the_window_running = false; //Close the window
                    }
                    break;
                default:
                    //This is wrong
                    LOG_ERR("game_ui::main_loop(): Unknow UI state, the event cannot be handled!");
                    break;
                };
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
        ELOG("game_ui::mouse_press_event(): Mouse button press, x:",event.mouseButton.x,"/y:",event.mouseButton.y);
        //Check if the mouse is moving in the gameplay area or over some menu ecc..
        if( is_over_the_game_map( event.mouseButton.x , event.mouseButton.y ) )
        {
            game_map::field_graphics_t* field = map->get_field_at_pos( event.mouseButton.x , event.mouseButton.y );
            if( field == nullptr )
            {
                write_info("No action available!");
                return;
            }
            //the user press the button over the game map
            //The only action allowed is to open the city map if any thus check if there's any city under this position.
            cities::city_agent* agent = field->manager->get_city_agent();
            if( agent )
            {
                //Ok.. Entering the city view.
                LOG("game_ui::mouse_press_event(): Entering the city id:",agent->get_city_id());
                current_city = agent;
                city_ui->set_cityagent( agent );
                current_view = type_of_view_t::city_map_view;
                city_ui->enter_city_menu();
            }
            else
            {
                write_info("No action available!");
            }
        }
        else
        {
            //The click seems to happen over a menu of over a info bar
            handle_console_click( event.mouseButton.x , event.mouseButton.y );
        }
    }

    //Manage the mouse moving event
    void game_ui::mouse_moving_event( const sf::Event& event )
    {
        //Check if the mouse is moving in the gameplay area or over some menu ecc..
        if( is_over_the_game_map( event.mouseMove.x , event.mouseMove.y ) )
        {
            game_map_mouse_move( event );
        }
        else
        {
            write_info("");
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
            //The message is different in case of the mouse moving over a city
            cities::city_agent* agent = field->manager->get_city_agent();
            if( agent != nullptr )
            {
                message << "City: " << agent->get_city_info()->name << std::endl;
            }
            else
            {
                message << "Field name: "<<field->descriptor->name <<", ID: "<<field->field->field_id<<"\nMouse coord: "<<event.mouseMove.x<<","<<event.mouseMove.y<<std::endl;
            }
            write_info( message.str() );
        }
        else
        {
            write_info( "Unknow position" );
        }

        message.str("");
    }

    //This function returns true if the user is moving over the playable area
    bool game_ui::is_over_the_game_map( long x_pos , long y_pos )
    {
        if( y_pos >= ui_config.map_canvas_setting.canvas_y_offset &&
            y_pos <= ( ui_config.map_canvas_setting.canvas_y_offset + ui_config.map_canvas_setting.canvas_height ) )
        {
            if( x_pos >= ui_config.map_canvas_setting.canvas_x_offset &&
                x_pos <= ( ui_config.map_canvas_setting.canvas_x_offset + ui_config.map_canvas_setting.canvas_width ) )
            {
                return true;
            }
        }
        return false;
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
        //Draw the status info.
        print_status_info();
        window.display();
    }

    void game_ui::print_status_info()
    {
        if( current_view == type_of_view_t::game_map_view )
        {
            write_status( "Game map" );
        }
        else if( current_view == type_of_view_t::city_map_view )
        {
            write_status( current_city->get_city_info()->name );
        }
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
        current_city->get_city_map()->draw_the_map( window );
        city_ui->draw_city_ui_elements();
    }
}






























