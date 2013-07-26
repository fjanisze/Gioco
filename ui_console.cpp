#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "ui_console.hpp"

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

    void ui_button_t::set_text( const std::string& text , sf::Font font  )
    {
    }

    void ui_button_t::set_appearence( const sf::Color& color )
    {
        vertex[0].color = color;
        vertex[0].color = color;
        vertex[0].color = color;
        vertex[0].color = color;
    }

    sf::VertexArray& ui_button_t::get_vertex()
    {
        return vertex;
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
        viewport_setting.map_x_offset = 0;
        viewport_setting.map_y_offset = 20;
        viewport_setting.map_width = 600;
        viewport_setting.map_height = 600;
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
            }
        }
        return result;
    }

    void console_wnd_t::set_font( sf::Font fnt )
    {
        text.setFont( fnt );
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
        ELOG("console_wnd_t::add_button(): Adding button element, ID:",button.get_id());
        buttons[ index ] = button;
        //Copy the vertex
        vertex.push_back( button.get_vertex() );
    }

    console_manager::console_manager( ) : building_mng( nullptr )
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
        info_console.create( 0 , window_config.viewport_setting.map_height + status_console_height , window_config.window_width , 30 );
        info_console.set_color( sf::Color( 10 , 20 , 30 ) );
        //Main console
        main_console.create( window_config.viewport_setting.map_width , status_console_height , 200 , 600 );
        main_console.set_color( sf::Color( 20 , 30 , 40 ) );
        //Create the buttons for the main console.
        main_console.add_button_map( main_menu_button_position , 10 );

        //Copy the font
        font = &window_config.font;
        status_console.set_font( *font );
        main_console.set_font( *font );
        info_console.set_font( *font );
    }

    //Draw the console in the proper context
    void console_manager::draw_console( sf::RenderWindow& window )
    {
        //Draw
    //    window.draw( status_console.get_background_vertex() );
     //   window.draw( info_console.get_background_vertex() );
    //    std::vector< sf::VertexArray >& main_vertex = main_console.get_vertex();
    ////    for( auto elem : main_vertex )
     //   {
     //       window.draw( elem );
     //   }
        //text
    //    window.draw( status_console.get_text() );
    //    window.draw( main_console.get_text() );
     //   window.draw( info_console.get_text() );
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
        if( point.is_console_point )
        {
            std::cout<<"Click\n";
        }
    }

    void console_manager::set_building_manager( buildings::building_manager* mng )
    {
        LOG("console_manager::set_building_manager(): Setting building manager, ADDR:",mng);
        building_mng = mng;
    }

    //Create the proper context for the main menu
    void console_manager::enter_main_manu()
    {
        if( building_mng == nullptr )
        {
            LOG_WARN("console_manager::enter_main_manu(): Cannot execute, building_mng is still null");
            return;
        }
        ELOG("console_wnd_t::enter_main_manu(): Entering main menu..");
        //As now, this menu just show the building which is possible to build
        graphic_elements::ui_button_t button;
        //One button for each building that can be built
        std::vector< buildings::appartment_descriptor_t* >* appartment = building_mng->get_all_the_appartment();
        if( appartment != nullptr )
        {
            long y_pos = 0;
            long button_id = 0;
            for( auto elem : (*appartment) )
            {
                button.create( 0 , y_pos , 200 , 60 );
                button.set_appearence( sf::Color::Black );
                button.set_text( elem->descriptor.name , *font );
                button.set_id( button_id );
                main_console.add_button( button , button_id );
                ++button_id;
                y_pos += 60;
            }
        }
        else
        {
            LOG_WARN("console_manager::enter_main_manu(): No appartment available for the menu!");
        }
    }

}



















