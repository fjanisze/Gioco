#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "ui_console.hpp"
#include "../game.hpp"

namespace graphic_elements
{

    ui_button_t::ui_button_t() : vertex( new sf::VertexArray( sf::Quads , 4 ) )
                                ,button_text( new sf::Text() )
                                ,reference_id(-1)
    {
    }

    ui_button_t::~ui_button_t()
    {
        LOG("ui_button_t::~ui_button_t(): Destroying");
    }

    void ui_button_t::create( long x_pos, long y_pos, long width, long height )
    {
        LOG("ui_button_t::create(): New button : x_pos:",x_pos,",y_pos:",y_pos,",width:",width,",height:",height);
        vertex.get()[0].position = sf::Vector2f( x_pos , y_pos );
        vertex.get()[1].position = sf::Vector2f( x_pos + width , y_pos );
        vertex.get()[2].position = sf::Vector2f( x_pos + width , y_pos + height );
        vertex.get()[3].position = sf::Vector2f( x_pos , y_pos + height );
    }

    void ui_button_t::set_offset( long x_axis, long y_axis )
    {
        button_text.lock();
        vertex.lock();
        for( short i = 0 ; i < 4 ; i++ )
        {
            vertex.get()[ i ].position += sf::Vector2f( x_axis , y_axis );
        }
        button_text.get().setPosition( vertex.get()[ 0 ].position );
        vertex.unlock();
        button_text.unlock();
    }

    void ui_button_t::set_text( const std::string& text )
    {
        button_text.lock();
        button_text.get().setPosition( vertex.get()[ 0 ].position );
        button_text.get().setString( text.c_str() );
        button_text.get().setCharacterSize( 12 );
        button_text.unlock();
    }

    sf::Text ui_button_t::get_text()
    {
        return button_text.get();
    }

    void ui_button_t::set_appearence( const sf::Color& color )
    {
        vertex.lock();
        vertex.get()[0].color = color;
        vertex.get()[1].color = color;
        vertex.get()[2].color = color;
        vertex.get()[3].color = color;
        vertex.unlock();
    }

    //Return true if the point is on this button
    bool ui_button_t::is_point_over_the_button( long x_pos , long y_pos )
    {
        sf::VertexArray& vrtx = vertex.get();
        if( ( x_pos >= vrtx[0].position.x ) && ( x_pos <= vrtx[1].position.x ) )
        {
            if( ( y_pos >= vrtx[0].position.y ) && ( y_pos <= vrtx[3].position.y ) )
            {
                return true;
            }
        }
        return false;
    }

    void ui_button_t::set_id( long id )
    {
        reference_id = id;
    }

    long ui_button_t::get_id() const
    {
        return reference_id;
    }

    void ui_button_t::set_action_id( long id )
    {
        action_id = id;
    }

    long ui_button_t::get_action_id()
    {
        return action_id;
    }

    //The button add to the provided graphic context all the information about its drawable objects
    void ui_button_t::add_to_graphic_context( long context_id )
    {
        LOG("ui_button_t::add_to_graphic_context(): Adding button ID:",reference_id,", to the graphic context ID",context_id);
        drawing_objects::drawing_facility* draw = drawing_objects::drawing_facility::get_instance();
        draw->add( &vertex , context_id );
        draw->add( &vertex , context_id );
    }
}

namespace graphic_ui
{
    ////////////////////////////////////////////////////////////////////
	//
	//
	//	Implementation for console_wnd_state and console_wnd_state_handler
	//
	//
	////////////////////////////////////////////////////////////////////

    console_manager::console_manager( )
    {
    }

    //The player just pushed the mouse button over a menu or status bar
    void console_manager::handle_console_click( long x_pos , long y_pos )
    {
        ELOG("console_manager::handle_console_click(): User click, x:",x_pos,",y:",y_pos );
        //Actually only clicks on the main console trigger some action

    }

    //This function is needed to trigger the proper action when a button is pressed
    void console_manager::button_trigger_action( graphic_elements::ui_button_t* button )
    {
        ELOG("console_manager::button_trigger_action(): Pressed the button ID: " , button->get_id() );
        switch( button->get_id() )
        {
        case COMMON_BUILD_BUTTON:
            {
            }
            break;
        case COMMON_MAP_BUTTON:
            {
                current_view = type_of_view_t::game_map_view;
                game_manager::game_manager::get_instance()->get_game_ui()->get_city_ui()->set_std_view_mode();
                show_map_main_menu();
            }
            break;
        case COMMON_QUIT_BUTTON: //The user want to quit the application
            {

            }
        default:
            {
            }
            break;
        };
        ELOG("console_manager::button_trigger_action(): Done.");
    }


    //Prepare the city menu
    void console_manager::show_city_main_menu()
    {
        ELOG("console_manager::show_city_main_menu(): Entering");
    }

    //Prepare the menu in the game map view.
    void console_manager::show_map_main_menu()
    {
        ELOG("console_manager::show_map_main_menu(): Entering..");
    }

    //Write a message in the info console
    void console_manager::write_info( const std::string& msg )
    {
    }

    //Used to update the status console
    void console_manager::write_status( const std::string& location )
    {
    }
}



















