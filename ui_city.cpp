#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "ui_city.hpp"

namespace city_ui_manager
{
    //Constructor
    city_ui::city_ui( sf::RenderWindow* rnd_window )
    {
        LOG("city_ui::city_ui(): Creating the city_ui object");
        focus_box = new(std::nothrow) sf::VertexArray( sf::LinesStrip , 5 );
        assert( focus_box != nullptr );
        window = rnd_window;
    }

    //Destructor
    city_ui::~city_ui()
    {
        delete focus_box;
    }

    //Before using the city_ui the proper city agent need to be provided
    void city_ui::set_cityagent( citymap::city_agent* agent )
    {
        ELOG("city_ui::set_cityagent(): Setting new city agent :",agent);
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
}
