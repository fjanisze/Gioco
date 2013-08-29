#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "ui_city.hpp"
#include "../buildings.hpp"
#include "../game.hpp"

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
        input_mode = city_ui_input_mode_t::view_mode;
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

    //Manage the mouse click when we are in the city view
    void city_ui::mouse_press_event( const sf::Event& event )
    {
        if(! is_over_the_game_map( event.mouseButton.x , event.mouseButton.y ) )
        {
            ui_console->handle_console_click( event.mouseButton.x , event.mouseButton.y );
        }
        else
        {
            if( input_mode == city_ui_input_mode_t::building_mode )
            {
                //If we are here, then the user want to build a construction on the map.
                build_info.field = city_agent->get_field_at_pos( event.mouseMove.x , event.mouseMove.y );
                //the building info structure should contain all the proper information now
                if( handle_new_construction() )
                {
                    //Quit the construction mode
                    set_std_view_mode();
                    update_focus_box( event.mouseButton.x , event.mouseButton.y );
                }
            }
        }
    }

    void city_ui::city_map_mouse_move( const sf::Event& event )
    {
        static std::stringstream message;
        static citymap::citymap_field_t* last_field = nullptr;
        //current_city should not be nullptr.
        if( city_agent != nullptr )
        {
            //Check for the field and print the information
            citymap::citymap_field_t* field = city_agent->get_field_at_pos( event.mouseMove.x , event.mouseMove.y );
            if( field != nullptr )
            {
                message << "Field name: "<<field->descriptor->name <<", ID: "<<field->field_id<<"\nMouse coord: "<<event.mouseMove.x<<","<<event.mouseMove.y<<std::endl;
                ui_console->write_info( message.str() );
                //Update the focus box, if the field is changed
                if( last_field == nullptr || ( last_field->field_id != field->field_id ) )
                {
                    update_focus_box( event.mouseMove.x , event.mouseMove.y );
                    last_field = field;
                }
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

        if( input_mode == city_ui_input_mode_t::view_mode )
        {
            (*focus_box) = sf::VertexArray( sf::LinesStrip , 5 );
            (*focus_box)[0].position = sf::Vector2f( x_pos , y_pos );
            (*focus_box)[1].position = sf::Vector2f( x_pos + field_width , y_pos );
            (*focus_box)[2].position = sf::Vector2f( x_pos + field_width , y_pos + field_height );
            (*focus_box)[3].position = sf::Vector2f( x_pos , y_pos + field_height );
            (*focus_box)[4].position = sf::Vector2f( x_pos , y_pos );

            for( short i = 0 ; i <= 4 ; i++ )
            {
                (*focus_box)[i].color = sf::Color::Red;
            }
        }
        else if( input_mode == city_ui_input_mode_t::building_mode )
        {
            (*focus_box) = sf::VertexArray( sf::Quads , 4 );
            (*focus_box)[0].position = sf::Vector2f( x_pos , y_pos );
            (*focus_box)[1].position = sf::Vector2f( x_pos + field_width , y_pos );
            (*focus_box)[2].position = sf::Vector2f( x_pos + field_width , y_pos + field_height );
            (*focus_box)[3].position = sf::Vector2f( x_pos , y_pos + field_height );
            //We should use the texture for the building here, but at this point no texture are available, so just use a color
            for( short i = 0 ; i <= 4 ; i++ )
            {
                (*focus_box)[i].color = sf::Color::Black;
            }
        }
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
    //This will trigger the capability check (is possible to build?) and some graphic changes.
    void city_ui::handle_build_btn_click( long action_id )
    {
        ELOG( "city_ui::handle_build_btn_click(): The user want to build a building, Action ID:",action_id,",city ID: ",city_agent->get_city_id() );
        //Set the building mode, now the user need to click on a proper field to trigger the construction
        set_building_mode();
        build_info.building_id = action_id;
    }

    //This function is called when the user has chosen which building want to build and has clicked on the map for the place
    bool city_ui::handle_new_construction()
    {
        ELOG("city_ui::handle_new_construction(): The user want to build a new building, building ID:",build_info.building_id,", field ID:", build_info.field->field_id,",city ID:",build_info.field->descriptor->obj_id );
        //Get the construction manager and try to build the building
        constructions::construction_manager* constr_mng = game_manager::game_manager::get_instance()->get_construction_manager();
        assert( constr_mng != nullptr );
        //Trigger the construction
        constructions::construction_handler_t handler = constr_mng->start_construction( build_info.building_id , city_agent->get_city_id() , build_info.field->field_id);
        return true;
    }

    //The view mode is also the 'default' viewing mode
    void city_ui::set_std_view_mode()
    {
        ELOG("city_ui::set_std_view_mode(): Setting the standard view mode");
        input_mode = city_ui_input_mode_t::view_mode;
    }

    void city_ui::set_building_mode()
    {
        ELOG("city_ui::set_view_mode(): Setting the building mode");
        input_mode = city_ui_input_mode_t::building_mode;
    }
}















