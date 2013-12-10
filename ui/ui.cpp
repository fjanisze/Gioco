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

        //Create the proper instance, check if this is the only one
        assert( instance == nullptr );
        instance = this;
        //Create the viewport
		map = game_map::game_map::get_instance();
		map->configure_game_canvas( ui_config.map_canvas_setting );

		//Create the city_ui object
		city_ui = new( std::nothrow ) city_ui_manager::city_ui( window , ui_config.map_canvas_setting );
		assert( city_ui != nullptr );

		//Set the console manager
		city_ui->set_console_manager( this );

		//Init the console
		init_consoles( ui_config );

		//Default view
		set_view_to_gamemap();
    }

    //Return the pointer to the city_ui
    city_ui_manager::city_ui* game_ui::get_city_ui()
    {
        return city_ui;
    }

    game_ui::~game_ui()
    {
    }

    //Main loop handling the graphic interface
    void game_ui::main_loop()
    {
        LOG("game_ui::main_loop(): Entering..");
  //      std::thread screen_refresh_thread{ &game_ui::screen_refresh, this }; TODO
        //Map Menu
        //show_map_main_menu();
        is_the_window_running = true;
        drawing_objects::drawing_facility* draw = drawing_objects::drawing_facility::get_instance();
        //Event fetching loop
        LOG("game_ui::main_loop(): Starting the loop");
        while( is_the_window_running )
        {
            sf::Event event;
            while( draw->poll_event( event ) )
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
            //screen_refresh(); TODO
        }
   //     screen_refresh_thread.join(); TODO
        //We are quitting, let's clean
        map->destroy_vertex_map();
        LOG("game_ui::main_loop(): Quitting..");
    }

    //The quit function terminate the game_ui object freeing all the used memory
    void game_ui::quit()
    {
        LOG("game_ui::quit(): Quitting!!");
        is_the_window_running = false;
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
                set_view_to_citymap();
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

    //Set the current view to the citymap view -entering the city-
    void game_ui::set_view_to_citymap()
    {
        ELOG("game_ui::set_view_to_citymap(): Set the citymap view");
        current_view = type_of_view_t::city_map_view;
    }

    //Set the current view to the game map -entering the map-
    void game_ui::set_view_to_gamemap()
    {
        ELOG("game_ui::set_view_to_gamemap(): Set the map view");
        current_view = type_of_view_t::game_map_view;
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

}






























