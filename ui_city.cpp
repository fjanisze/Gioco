#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "ui_city.hpp"

namespace city_ui_manager
{
    //Constructor
    city_ui::city_ui( sf::RenderWindow* rnd_window , game_map::map_viewport_settings_t map_setting)
    {
        LOG("city_ui::city_ui(): Creating the city_ui object");
        focus_box = new(std::nothrow) sf::VertexArray( sf::LinesStrip , 5 );
        assert( focus_box != nullptr );
        window = rnd_window;
        map_view_setting = map_setting;
    }

    //Destructor
    city_ui::~city_ui()
    {
        delete focus_box;
    }

    //Before using the city_ui the proper city agent need to be provided
    void city_ui::set_cityagent( cities::city_agent* agent )
    {
        ELOG("city_ui::set_cityagent(): Setting new city agent:",agent);
        city_agent = agent;
        field_width = city_agent->get_city_info()->citymap->get_field_width();
        field_height = city_agent->get_city_info()->citymap->get_field_height();
    }

    void city_ui::set_console_manager( graphic_ui::console_manager* console )
    {
        ELOG("city_ui::set_console_manager(): Setting the console manager: ",console);
        ui_console = console;
    }

    //Handle the events in the city interface
    long city_ui::handle_event( const sf::Event& event )
    {
        switch( event.type )
        {
        case sf::Event::Closed:
            return -1;
        case sf::Event::MouseMoved:
            //The mouse is moving over the game window.
            city_map_mouse_move( event );
            break;
        case sf::Event::MouseButtonPressed:
            mouse_press_event( event );
            break;
        default:
            break;
        };
        return 0;
    }

    //This function returns true if the user is moving over the playable area
    bool city_ui::is_over_the_game_map( long x_pos , long y_pos )
    {
        if( y_pos >= map_view_setting.map_y_offset &&
            y_pos <= ( map_view_setting.map_y_offset + map_view_setting.map_height ) )
        {
            if( x_pos >= map_view_setting.map_x_offset &&
                x_pos <= ( map_view_setting.map_x_offset + map_view_setting.map_width ) )
            {
                return true;
            }
        }
        return false;
    }

    void city_ui::mouse_press_event( const sf::Event& event )
    {
        if(! is_over_the_game_map( event.mouseButton.x , event.mouseButton.y ) )
        {
            ui_console->handle_console_click( event.mouseButton.x , event.mouseButton.y );
        }
    }

    void city_ui::city_map_mouse_move( const sf::Event& event )
    {
        static std::stringstream message;
        //current_city should not be nullptr.
        if( city_agent != nullptr )
        {
            //Check for the field and print the information
            citymap::citymap_field_t* field = city_agent->get_field_at_pos( event.mouseMove.x , event.mouseMove.y );
            if( field != nullptr )
            {
                message << "Field name: "<<field->descriptor->name <<", ID: "<<field->field_id<<"\nMouse coord: "<<event.mouseMove.x<<","<<event.mouseMove.y<<std::endl;
                ui_console->write_info( message.str() );
                //Update the focus box;
                update_focus_box( event.mouseMove.x , event.mouseMove.y );
            }
            else
            {
                ui_console->write_info("Moving over the menu.");
            }
        }
        else
        {
            LOG_ERR("city_ui::city_map_mouse_move(): city_agent is nullptr, this shouldn't happen");
        }
        message.str("");
    }

    //Check the position of the mouse and update the focus box
    void city_ui::update_focus_box( long x_pos , long y_pos )
    {
        x_pos -= ( x_pos % field_width );
        y_pos -= ( y_pos % field_height );

        //add Offset
        x_pos += map_view_setting.map_x_offset;
        y_pos += map_view_setting.map_y_offset;

        (*focus_box)[0].position = sf::Vector2f( x_pos , y_pos );
        (*focus_box)[1].position = sf::Vector2f( x_pos + field_width , y_pos );
        (*focus_box)[2].position = sf::Vector2f( x_pos + field_width , y_pos + field_height );
        (*focus_box)[3].position = sf::Vector2f( x_pos , y_pos + field_height );
        (*focus_box)[4].position = sf::Vector2f( x_pos , y_pos );
        (*focus_box)[0].color = sf::Color::Red;
        (*focus_box)[1].color = sf::Color::Red;
        (*focus_box)[2].color = sf::Color::Red;
        (*focus_box)[3].color = sf::Color::Red;
        (*focus_box)[4].color = sf::Color::Red;
    }

    //Draw other elements like the focus box ecc
    void city_ui::draw_city_ui_elements()
    {
        if( focus_box )
        {
            window->draw( *focus_box );
        }
    }

    //When the user enter the city menu
    void city_ui::enter_city_menu()
    {
        ELOG("city_ui::enter_city_menu(): Entering.");
        ui_console->show_city_main_menu();
    }

    //This function is called when the user click on a button related with a building that may be built
    void city_ui::handle_build_btn_click( long action_id )
    {
        ELOG("city_ui::handle_build_btn_click(): Action ID:",action_id,",city ID: ",city_agent->get_city_id());
    }
}















