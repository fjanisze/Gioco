#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "ui_city.hpp"
#include "../buildings.hpp"
#include "../game.hpp"

namespace city_ui_manager
{
    //Constructor
    city_ui::city_ui( game_map::game_canvas_settings_t game_canvas_setting )
    {
        LOG("city_ui::city_ui(): Creating the city_ui object");
        LOG("city_ui::city_ui(): canvas: X size:",game_canvas_setting.canvas_width,",Y size: ",game_canvas_setting.canvas_height,",X offset: ",game_canvas_setting.canvas_x_offset,", Y offset: ",game_canvas_setting.canvas_y_offset);
        game_canvas = game_canvas_setting;
        input_mode = city_ui_input_mode_t::view_mode;
        //Create the focus box
        sf::VertexArray* vertex = new(std::nothrow) sf::VertexArray( sf::LinesStrip , 5 );
        assert( vertex != nullptr );
        focus_box.update( vertex );
        focus_box.set_render_state( drawing_objects::draw_obj_render_state::render );
    }

    //Destructor
    city_ui::~city_ui()
    {
        LOG("city_ui::~city_ui(): Destroying the city_ui object.");
    }

    //Before using the city_ui the proper city agent need to be provided
    void city_ui::set_cityagent( cities::city_agent* agent )
    {
        ELOG("city_ui::set_cityagent(): Setting new city agent:",agent);
        city_agent = agent;
        field_width = city_agent->get_city_info()->citymap->get_field_width();
        field_height = city_agent->get_city_info()->citymap->get_field_height();
        //Add the focus box to the graphic context for the city
        int city_graphic_contex_it = city_agent->get_city_map()->get_graphic_context_id();
        drawing_objects::drawing_facility::get_instance()->add( &focus_box , city_graphic_contex_it );
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
            {
                ELOG("city_ui::handle_event(): User want to close the application.");
            }
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
        ELOG("city_ui::is_over_the_game_map(): x:",x_pos,",y:",y_pos);
        if( y_pos >= game_canvas.canvas_y_offset &&
            y_pos < ( game_canvas.canvas_y_offset + game_canvas.canvas_height ) )
        {
            if( x_pos >= game_canvas.canvas_x_offset &&
                x_pos < ( game_canvas.canvas_x_offset + game_canvas.canvas_width ) )
            {
                ELOG("city_ui::is_over_the_game_map(): The point is over the game map.");
                return true;
            }
        }
        return false;
    }

    //Manage the mouse click when we are in the city view
    bool city_ui::mouse_press_event( const sf::Event& event )
    {
        bool operation_verdict = true;
        ELOG("city_ui::mouse_press_event(): Mouse button pressed, x:",event.mouseButton.x,",y:",event.mouseButton.y);
        if(! is_over_the_game_map( event.mouseButton.x , event.mouseButton.y ) )
        {
            ui_console->handle_console_click( event.mouseButton.x , event.mouseButton.y );
        }
        else
        {

        }
        return operation_verdict;
    }

    //This function is called when the mouse move event is triggered over the city map
    void city_ui::city_map_mouse_move( const sf::Event& event )
    {
        static std::stringstream message;
        static citymap::citymap_field_t* last_field = nullptr;
        //Remove the offset to have coherent data.
        long x = event.mouseMove.x ,
            y = event.mouseMove.y;
        //current_city should not be nullptr.
        if( city_agent != nullptr )
        {
            //Check for the field and print the information
            citymap::citymap_field_t* field = city_agent->get_field_at_pos( x , y );
            if( field != nullptr )
            {
                message << "Field name: "<<field->descriptor->name <<", ID: "<<field->field_id<<",Coord x:"<<event.mouseMove.x<<",y:"<<event.mouseMove.y<<std::endl;
                //Is there any construction on the field?
                if( field->construction != nullptr )
                {
                    //Print something about the construction.
                    message<<"Construction: "<<field->construction->get_name()<<", with ID:"<<field->construction->get_obj_id()<<std::endl;
                }
                ui_console->write_info( message.str() );
                //Update the focus box
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
        x_pos -= ( ( x_pos - game_canvas.canvas_x_offset ) % field_width );
        y_pos -= ( ( y_pos - game_canvas.canvas_y_offset ) % field_height );

        static long old_x_pos{} , old_y_pos{};

        if( x_pos == old_x_pos && y_pos == old_y_pos )
        {
            return; //Nothing to update here
        }

        old_x_pos = x_pos;
        old_y_pos = y_pos;

        if( input_mode == city_ui_input_mode_t::view_mode )
        {
            focus_box.lock();
            focus_box.get() = sf::VertexArray( sf::LinesStrip , 5 );
            focus_box.get()[0].position = sf::Vector2f( x_pos , y_pos );
            focus_box.get()[1].position = sf::Vector2f( x_pos + field_width , y_pos );
            focus_box.get()[2].position = sf::Vector2f( x_pos + field_width , y_pos + field_height );
            focus_box.get()[3].position = sf::Vector2f( x_pos , y_pos + field_height );
            focus_box.get()[4].position = sf::Vector2f( x_pos , y_pos );

            for( short i = 0 ; i <= 4 ; i++ )
            {
                focus_box.get()[i].color = sf::Color::Red;
            }
            focus_box.unlock();
        }
        else if( input_mode == city_ui_input_mode_t::building_mode )
        {
            focus_box.lock();
            focus_box.get() = sf::VertexArray( sf::Quads , 4 );
            focus_box.get()[0].position = sf::Vector2f( x_pos , y_pos );
            focus_box.get()[1].position = sf::Vector2f( x_pos + field_width , y_pos );
            focus_box.get()[2].position = sf::Vector2f( x_pos + field_width , y_pos + field_height );
            focus_box.get()[3].position = sf::Vector2f( x_pos , y_pos + field_height );
            //We should use the texture for the building here, but at this point no texture are available, so just use a color
            for( short i = 0 ; i <= 4 ; i++ )
            {
                focus_box.get()[i].color = sf::Color::Black;
            }
            focus_box.unlock();
        }
    }

    //When the user enter the city menu
    void city_ui::enter_city()
    {
        LOG("city_ui::enter_city(): Entering.");
        ui_console->show_city_main_menu();
        //Enable the graphic context for the city
        int graphic_context_id = city_agent->get_city_map()->get_graphic_context_id();
        drawing_objects::drawing_facility::get_instance()->enable_context( graphic_context_id );
    }

    //When the user is leaving the city UI
    void city_ui::quit_city()
    {
        LOG("city_ui::quit_city(): Quitting.");
        int graphic_context_id = city_agent->get_city_map()->get_graphic_context_id();
        drawing_objects::drawing_facility::get_instance()->disable_context( graphic_context_id );
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















