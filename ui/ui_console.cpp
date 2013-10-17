#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "ui_console.hpp"
#include "../game.hpp"

namespace graphic_elements
{

    ui_button_t::ui_button_t() : vertex( sf::Quads , 4 ) , reference_id(-1)
    {
    }

    void ui_button_t::create( long x_pos, long y_pos, long width, long height )
    {
        LOG("ui_button_t::create(): New button : x_pos:",x_pos,",y_pos:",y_pos,",width:",width,",height:",height);
        vertex[0].position = sf::Vector2f( x_pos , y_pos );
        vertex[1].position = sf::Vector2f( x_pos + width , y_pos );
        vertex[2].position = sf::Vector2f( x_pos + width , y_pos + height );
        vertex[3].position = sf::Vector2f( x_pos , y_pos + height );
    }

    void ui_button_t::set_offset( long x_axis, long y_axis )
    {
        for( short i = 0 ; i < 4 ; i++ )
        {
            vertex[ i ].position += sf::Vector2f( x_axis , y_axis );
        }
        button_text.setPosition( vertex[ 0 ].position );
    }

    void ui_button_t::set_text( const std::string& text , const sf::Font* font  )
    {
        button_text.setPosition( vertex[ 0 ].position );
        button_text.setFont( *font );
        button_text.setString( text.c_str() );
        button_text.setCharacterSize( 12 );
    }

    sf::Text  ui_button_t::get_text()
    {
        return button_text;
    }

    void ui_button_t::set_appearence( const sf::Color& color )
    {
        vertex[0].color = color;
        vertex[1].color = color;
        vertex[2].color = color;
        vertex[3].color = color;
    }

    sf::VertexArray& ui_button_t::get_vertex()
    {
        return vertex;
    }

    //Return true if the point is on this button
    bool ui_button_t::is_point_over_the_button( long x_pos , long y_pos )
    {
        if( ( x_pos >= vertex[0].position.x ) && ( x_pos <= vertex[1].position.x ) )
        {
            if( ( y_pos >= vertex[0].position.y ) && ( y_pos <= vertex[3].position.y ) )
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

    long ui_button_t::get_id()
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
}

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
        window_height = 650;
        window_width = 800;
        //Game viewport
        map_canvas_setting.canvas_x_offset = 0;
        map_canvas_setting.canvas_y_offset = 20;
        map_canvas_setting.canvas_width = 600;
        map_canvas_setting.canvas_height = 600;
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
        background_vertex = sf::VertexArray( sf::Quads , 4 );
        //Create the vertex position
        background_vertex[ 0 ].position = sf::Vector2f( x_off, y_off );
        background_vertex[ 1 ].position = sf::Vector2f( x_off + width , y_off );
        background_vertex[ 2 ].position = sf::Vector2f( x_off + width , y_off + height );
        background_vertex[ 3 ].position = sf::Vector2f( x_off , y_off + height );
        //Set a default color
        set_color( sf::Color( 10 , 30 , 150 ) );
        //Configure properly the text entity
        text.setPosition( background_vertex[ 0 ].position );
        text.setCharacterSize( 12 );
    }

    void console_wnd_t::set_color( sf::Color color )
    {
        for( short i = 0 ; i < 4 ; i++  )
        {
            background_vertex[ i ].color = color;
        }
    }

    //Return a console_point_t related with the specified point
    console_point_t console_wnd_t::over_the_console( long x , long y)
    {
        console_point_t result;
        if( x > x_offset && x < ( width + x_offset ) )
        {
            if( y > y_offset && y < ( height + y_offset ) )
            {
                //This point is over this console.
                result.x_pos = x;
                result.y_pos = y;
                result.is_console_point = true;
                //Check if the click happened over a button
                for( auto& elem : buttons )
                {
                    if( elem.second.is_point_over_the_button( x, y ) )
                    {
                        result.click_possible = true;
                        result.button = &elem.second;
                        break;
                    }
                }
            }
        }
        return result;
    }

    void console_wnd_t::set_font( const sf::Font* fnt )
    {
        text.setFont( *fnt );
    }

    std::vector< sf::VertexArray >& console_wnd_t::get_vertex()
    {
        return vertex;
    }

    sf::VertexArray& console_wnd_t::get_background_vertex()
    {
        return background_vertex;
    }

    sf::Text& console_wnd_t::get_text()
    {
        return text;
    }

    //Get from the array all the position of the buttons
    void console_wnd_t::add_button_map( const button_position_t* map , short amount_of_buttons )
    {
        ELOG("console_wnd_t::add_button_map(): Num of buttons:",amount_of_buttons,",map addr:",map);
        for( short i = 0 ; i < amount_of_buttons ; i++ )
        {
            button_map.push_back( map[i] );
        }
    }

    //The value 'index' is used to set the button position on the base of the button position map provided
    void console_wnd_t::add_button( graphic_elements::ui_button_t button , short index )
    {
        std::string button_name = button.get_text().getString();
        ELOG("console_wnd_t::add_button(): Adding button element, ID:",button.get_id() , ",name: ",button_name );
        //Set the proper offset
        button.set_offset( x_offset , y_offset );
        buttons[ index ] = button;
    }

    void console_wnd_t::draw( sf::RenderWindow& window )
    {
        //Draw the vertex
        window.draw( background_vertex );
        for( auto elem : vertex )
        {
            window.draw( elem );
        }
        //Draw the button text
        for( auto elem : buttons )
        {
            window.draw( elem.second.get_vertex() );
            window.draw( elem.second.get_text() );
        }
        //Draw text
        window.draw( text );
    }

    void console_wnd_t::remove_all_buttons()
    {
        buttons.clear();
    }

    console_manager::console_manager( )
    {
    }

    //Initializate the console window
    short console_manager::init_consoles( const game_window_config_t& window_config )
    {
        std::lock_guard< std::mutex > lock( mutex );
        LOG("console_manager::init_consoles(): Initializing all the consoles");
        long status_console_height = 20;
        //Status console
        status_console.create( 0 , 0 , window_config.window_width , status_console_height );
        status_console.set_color( sf::Color( 255 , 100 , 20 ) );
        //Info Console
        info_console.create( 0 , window_config.map_canvas_setting.canvas_height + status_console_height , window_config.window_width , 30 );
        info_console.set_color( sf::Color( 10 , 20 , 30 ) );
        //Main console
        main_console.create( window_config.map_canvas_setting.canvas_width , status_console_height , 200 , 600 );
        main_console.set_color( sf::Color( 20 , 30 , 40 ) );
        //Create the buttons for the main console.
        main_console.add_button_map( main_menu_button_position , 10 );

        //Copy the font
        font = &window_config.font;
        status_console.set_font( font );
        main_console.set_font( font );
        info_console.set_font( font );
    }

    //Draw the console in the proper context
    void console_manager::draw_console( sf::RenderWindow& window )
    {
        status_console.draw( window );
        info_console.draw( window );
        main_console.draw( window);
    }

    //Write a message in the info console
    void console_manager::write_info( const std::string& msg )
    {
        info_console.get_text().setString( msg );
    }

    //Used to update the status console
    void console_manager::write_status( const std::string& location )
    {
        std::stringstream str;
        str << "View on: " << location;
        status_console.get_text().setString( str.str() );
    }

    //The player just pushed the mouse button over a menu or status bar
    void console_manager::handle_console_click( long x_pos , long y_pos )
    {
        ELOG("console_manager::handle_console_click(): User click, x:",x_pos,",y:",y_pos );
        //Actually only clicks on the main console trigger some action
        console_point_t point = main_console.over_the_console( x_pos , y_pos );
        if( point.is_console_point ) //Over the main console.
        {
            if( point.click_possible )
            {
                button_trigger_action( point.button );
            }
        }
    }

    //This function is needed to trigger the proper action when a button is pressed
    void console_manager::button_trigger_action( graphic_elements::ui_button_t* button )
    {
        ELOG("console_manager::button_trigger_action(): Pressed the button ID: " , button->get_id() );
        switch( button->get_id() )
        {
        case COMMON_BUILD_BUTTON:
            {
                //Add the build buttons.
                add_building_construction_btn( main_console , 90 );
            }
            break;
        case COMMON_MAP_BUTTON:
            {
                current_view = type_of_view_t::game_map_view;
                game_manager::game_manager::get_instance()->get_game_ui()->get_city_ui()->set_std_view_mode();
                show_map_main_menu();
            }
            break;
        default:
            {
                //Check for non common buttons, like the button the construction.
                if( !handle_non_common_button( button ) )
                {
                    LOG_WARN("console_manager::button_trigger_action(): Cannot trigger any action for the button ID: ",button->get_id() );
                }
            }
            break;
        };
    }

    //This function is used to handle a click of a non common button
    bool console_manager::handle_non_common_button( graphic_elements::ui_button_t* button  )
    {
        long button_id = button->get_id();
        ELOG("console_manager::handle_non_common_button(): Button ID: ", button_id );
        //Want the user build a new constuction?
        if( ( button_id >= BUILDING_BUTTON_ID_BEGIN ) && ( button_id < BUILDING_BUTTON_ID_END ) )
        {
            city_ui_manager::city_ui* city_ui = game_manager::game_manager::get_instance()->get_game_ui()->get_city_ui();
            city_ui->handle_build_btn_click( button->get_action_id() );
            return true;
        }
        return false;
    }

    //Create the proper context for the main menu
    void console_manager::add_building_construction_btn( console_wnd_t& console , long y_pos )
    {
        ELOG("console_wnd_t::add_building_construction_btn(): Adding the construction buttons..");
        constructions::construction_manager* constructions_mng = game_manager::game_manager::get_instance()->get_buildings();
        //As now, this menu just show the building which is possible to build
        graphic_elements::ui_button_t button;
        //One button for each building that can be built
        std::vector< constructions::construction_t* >* appartment = constructions_mng->get_all_construction();
        if( appartment != nullptr )
        {
            if( appartment->empty() )
            {
                LOG_ERR("console_manager::add_building_construction_btn(): No construction available, this is wrong..");
                return; //quit
            }
            long button_id = BUILDING_BUTTON_ID_BEGIN; //From 1000 begins the ID for the consturction buttons.
            for( auto elem : (*appartment) )
            {
                button.create( 0 , y_pos , 200 , 30 );
                button.set_appearence( sf::Color::Black );
                button.set_text( elem->get_name() , font );
                button.set_id( button_id );
                button.set_action_id( elem->get_obj_id() );
                console.add_button( button , button_id );
                ++button_id;
                y_pos += 30;
            }
        }
        else
        {
            LOG_WARN("console_manager::add_building_construction_btn(): No appartment available for the menu!");
        }
    }

    //Add to the main menu some common buttons
    long console_manager::add_city_common_btn( console_wnd_t& console )
    {
        ELOG("console_manager::add_city_common_btn(): Entering");

        graphic_elements::ui_button_t button;
        button.create( 0 , 0 , 100 , 60 );
        button.set_appearence( sf::Color::Blue );
        button.set_text( "Build" , font );
        button.set_id( COMMON_BUILD_BUTTON );
        console.add_button( button , COMMON_BUILD_BUTTON); //Back button, ID: 0


        button.create( 100 , 0 , 100 , 60 );
        button.set_appearence( sf::Color::Blue );
        button.set_text( "MAP" , font );
        button.set_id( COMMON_MAP_BUTTON );
        console.add_button( button , COMMON_MAP_BUTTON ); //MAP button, ID: 1

        return 60;
    }

    //Add to the main menu the common buttons which are visible in the game map view.
    long console_manager::add_map_common_btn( console_wnd_t& console )
    {
        //No button in the map view now.
        return 0;
    }

    //Prepare the city menu
    void console_manager::show_city_main_menu()
    {
        ELOG("console_manager::show_city_main_menu(): Entering");
        main_console.remove_all_buttons();
        long last_offset = add_city_common_btn( main_console );
    }

    //Prepare the menu in the game map view.
    void console_manager::show_map_main_menu()
    {
        ELOG("console_manager::show_map_main_menu(): Entering..");
        main_console.remove_all_buttons();
    }
}



















