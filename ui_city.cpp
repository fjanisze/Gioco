#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "ui_city.hpp"

namespace city_ui_manager
{
    //Constructor
    city_ui::city_ui()
    {
        LOG("city_ui::city_ui(): Creating the city_ui object");
    }

    //Before using the city_ui the proper city agent need to be provided
    void city_ui::set_cityagent( citymap::city_agent* agent )
    {
        ELOG("city_ui::set_cityagent(): Setting new city agent :",agent);
        city_agent = agent;
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
           // mouse_press_event( event );
            break;
        default:
            break;
        };
        return 0;
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

}
