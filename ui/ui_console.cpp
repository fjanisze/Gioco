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

        drawing_objects::drawing_facility::get_instance()->add( &vertex );
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
        background.update( new sf::VertexArray( sf::Quads , 4 ) );
        //Create the vertex position
        background.get()[ 0 ].position = sf::Vector2f( x_off, y_off );
        background.get()[ 1 ].position = sf::Vector2f( x_off + width , y_off );
        background.get()[ 2 ].position = sf::Vector2f( x_off + width , y_off + height );
        background.get()[ 3 ].position = sf::Vector2f( x_off , y_off + height );
        //Set a default color
        set_color( sf::Color( 10 , 30 , 150 ) );
        //Configure properly the text entity
        text.update( new sf::Text() );
        text.get().setPosition( sf::Vector2f( x_off, y_off ) );
        text.get().setCharacterSize( 12 );
        text.get().setColor( sf::Color::White );

        drawing_objects::drawing_facility::get_instance()->add( &background );
        drawing_objects::drawing_facility::get_instance()->add( &text );
    }

    void console_wnd_t::set_color( sf::Color color )
    {
        background.lock();
        for( short i = 0 ; i < 4 ; i++  )
        {
            background.get()[ i ].color = color;
        }
        background.unlock();
    }

    std::string console_wnd_t::get_text()
    {
        return text.get().getString();
    }

    void console_wnd_t::set_text( const std::string& msg )
    {
        text.lock();
        text.get().setString( msg.c_str() );
        text.unlock();
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
                    if( elem.second->is_point_over_the_button( x, y ) )
                    {
                        result.click_possible = true;
                        result.button = elem.second;
                        break;
                    }
                }
            }
        }
        return result;
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
    int console_wnd_t::add_button( std::shared_ptr< graphic_elements::ui_button_t >& button , short index )
    {
        std::string button_name = button->get_text().getString();
        ELOG("console_wnd_t::add_button(): Adding button element, ID:",button->get_id() , ",name: ",button_name );
        //If the button already exist, do not add
        for( auto& elem : buttons )
        {
            if( elem.second->get_id() == button->get_id() )
            {
                ELOG("console_wnd_t::add_button(): This button (",button->get_id(),") already exist, not adding");
                return -1; //Do not add this button, already exist.
            }
        }
        //Set the proper offset
        button->set_offset( x_offset , y_offset );
        buttons[ index ] = std::move( button );
        return index;
    }

    void console_wnd_t::remove_all_buttons()
    {
        ELOG("console_wnd_t::remove_all_buttons(): Cleaning the button container, container size: " , buttons.size() );
        if( buttons.size() )
        {
            buttons.clear();
        }
        ELOG("console_wnd_t::remove_all_buttons(): Done");
    }

    console_manager::console_manager( )
    {
    }

    //Initializate the console window
    std::vector< console_wnd_t* > console_manager::init_consoles( const game_window_config_t& window_config )
    {
        std::lock_guard< std::mutex > lock( mutex );
        LOG("console_manager::init_consoles(): Initializing all the consoles");
        long status_console_height = 20;
        //Put them in the vector and return back
        std::vector< console_wnd_t* > consoles;

        //Main console
        main_console.create( window_config.map_canvas_setting.canvas_width , status_console_height , 200 , 600 );
        main_console.set_color( sf::Color( 20 , 30 , 40 ) );
        consoles.push_back( &main_console );
        //Status console
        status_console.create( 0 , 0 , window_config.window_width , status_console_height );
        status_console.set_color( sf::Color( 255 , 100 , 20 ) );
        consoles.push_back( &status_console );
        //Info Console
        info_console.create( 0 , window_config.map_canvas_setting.canvas_height + status_console_height , window_config.window_width , 30 );
        info_console.set_color( sf::Color( 10 , 20 , 30 ) );
        consoles.push_back( &info_console );

        //Create the buttons for the main console.
        main_console.add_button_map( main_menu_button_position , 10 );

        return consoles;
    }

    //Write a message in the info console
    void console_manager::write_info( const std::string& msg )
    {
        info_console.set_text( msg );
    }

    //Used to update the status console
    void console_manager::write_status( const std::string& location )
    {
        std::stringstream str;
        str << "View on: " << location;
        status_console.set_text( str.str() );
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
                button_trigger_action( point.button.get() );
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
        case COMMON_QUIT_BUTTON: //The user want to quit the application
            {

            }
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
        ELOG("console_manager::button_trigger_action(): Done.");
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
    short console_manager::add_building_construction_btn( console_wnd_t& console , long y_pos )
    {
        ELOG("console_wnd_t::add_building_construction_btn(): Adding the construction buttons..");
        constructions::construction_manager* constructions_mng = game_manager::game_manager::get_instance()->get_buildings();
        //As now, this menu just show the building which is possible to build
        //One button for each building that can be built
        std::vector< constructions::construction_t* >* constructions = constructions_mng->get_all_construction();
        //Return the amount of buttons
        short amount_of_buttons = 0;
        if( constructions != nullptr )
        {
            if( constructions->empty() )
            {
                LOG_ERR("console_manager::add_building_construction_btn(): No construction available, this is wrong..");
                return amount_of_buttons;//quit
            }
            long button_id = BUILDING_BUTTON_ID_BEGIN; //From 1000 begins the ID for the consturction buttons.
            for( auto elem : (*constructions) )
            {
                std::shared_ptr< graphic_elements::ui_button_t > button( new graphic_elements::ui_button_t );
                button->create( 0 , y_pos , 200 , 30 );
                button->set_appearence( sf::Color::Black );
                button->set_text( elem->get_name() );
                button->set_id( button_id );
                button->set_action_id( elem->get_obj_id() );
                console.add_button( button , button_id );
                ++button_id;
                y_pos += 30;
                ++amount_of_buttons;
            }
        }
        else
        {
            LOG_WARN("console_manager::add_building_construction_btn(): No constructions available for the menu!");
        }
        return amount_of_buttons;
    }

    //Add to the main menu some common buttons
    long console_manager::add_city_common_btn( console_wnd_t& console )
    {
        ELOG("console_manager::add_city_common_btn(): Entering");

        std::shared_ptr< graphic_elements::ui_button_t > build_button( new graphic_elements::ui_button_t );
        build_button->create( 0 , 0 , 100 , 60 );
        build_button->set_appearence( sf::Color::Blue );
        build_button->set_text( "Build" );
        build_button->set_id( COMMON_BUILD_BUTTON );
        console.add_button( build_button, COMMON_BUILD_BUTTON ); //Back button, ID: 0

        std::shared_ptr< graphic_elements::ui_button_t > map_button( new graphic_elements::ui_button_t );
        map_button->create( 100 , 0 , 100 , 60 );
        map_button->set_appearence( sf::Color::Blue );
        map_button->set_text( "MAP" );
        map_button->set_id( COMMON_MAP_BUTTON );
        console.add_button( map_button , COMMON_MAP_BUTTON ); //MAP button, ID: 1

        return 60;
    }

    //Add to the main menu the common buttons which are visible in the game map view.
    long console_manager::add_map_common_btn( console_wnd_t& console )
    {
        ELOG("console_manager::add_map_common_btn(): Entering");
        std::shared_ptr< graphic_elements::ui_button_t > quit_button( new graphic_elements::ui_button_t );
        quit_button->create( 0 , 0 , 100 , 60 );
        quit_button->set_appearence( sf::Color::Blue );
        quit_button->set_text( "Quit" );
        quit_button->set_id( COMMON_QUIT_BUTTON );
        console.add_button( quit_button , COMMON_QUIT_BUTTON );

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
        add_map_common_btn( main_console );
    }
}



















