#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#ifndef UI_CONSOLE_HPP_INCLUDED
#define UI_CONSOLE_HPP_INCLUDED

#include "logging/logger.hpp"
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "ui_common.hpp"
#include "buildings.hpp"
#include <sstream>
#include <string>
#include <map>
#include <mutex>

namespace graphic_elements
{
    //Button entity, the user can click to trigger some action
    class ui_button_t
    {
        sf::VertexArray vertex; //Graphic appearence
        long reference_id;
    public:
        ui_button_t();
        void create( long x_pos, long y_pos, long width, long height );
        void set_text( const std::string& text , sf::Font font );
        void set_appearence( const sf::Color& color );
        sf::VertexArray& get_vertex();
        void set_id( long id )
        {
            reference_id = id;
        }
        long get_id()
        {
            return reference_id;
        }
    };
}

namespace graphic_ui
{
    struct button_position_t
    {
        long x,
             y;
        long width,
             height;
    };

    ////////////////////////////////////////////////////////////////////
	//
	//
	//	Button positions for all the menu, not more than 10 buttons are allowed at once in the main menu.
	//
	//
	////////////////////////////////////////////////////////////////////

    static const button_position_t main_menu_button_position[ 10 ] =
    {
        { 0 , 0 , 200 , 60 },
        { 0 , 60 , 200 , 60 },
        { 0 , 120 , 200 , 60 },
        { 0 , 180 , 200 , 60 },
        { 0 , 240 , 200 , 60 },
        { 0 , 300 , 200 , 60 },
        { 0 , 360 , 200 , 60 },
        { 0 , 420 , 200 , 60 },
        { 0 , 480 , 200 , 60 },
        { 0 , 540 , 200 , 60 }
    };

    ////////////////////////////////////////////////////////////////////
	//
	//
	//	console_point_t, console_wnd_t and the console manager
	//
	//
	////////////////////////////////////////////////////////////////////

    struct console_point_t
    {
        bool is_console_point; //True it the point is over the console
        long x_pos, y_pos;
        bool click_possible; //Is true if is possible to click the point to trigger some action
        long action_id; //Id related with the action triggered by the click
        console_point_t() : is_console_point( false ) ,
                            x_pos( -1 ) , y_pos( -1 ),
                            click_possible( false ),
                            action_id( -1 )
        {
        }
    };
    //Console Window information
    class console_wnd_t
    {
        //Those values specify where the top left corner of the console is places
        long x_offset,
            y_offset;
        //Size
        long width,
            height;
        //Related vertex
        sf::VertexArray background_vertex;
        std::vector< sf::VertexArray > vertex;
        //Text entity related with this console
        sf::Text text;
        //Position of all the buttons
        std::vector< button_position_t > button_map;
        std::map< long , graphic_elements::ui_button_t > buttons; //Button on the console
    public:
        //Constructor and utility
        console_wnd_t( long x_off , long y_off , long wnd_width, long wnd_height );
        console_wnd_t();
        void create( long x_off , long y_off , long wnd_width, long wnd_height );
        void set_color( sf::Color color );
        console_point_t over_the_console( long x , long y);
        void set_font( sf::Font fnt );
        std::vector< sf::VertexArray >& get_vertex();
        sf::VertexArray& get_background_vertex();
        sf::Text& get_text();
        void add_button_map( const button_position_t* map , short amount_of_buttons );
        void add_button( graphic_elements::ui_button_t button , short index );
    };

    //This object is reponsible for the console management
    class console_manager
    {
    protected:
        std::mutex mutex;
    private:
        //We assume that only two console are provided at the beginning
        console_wnd_t status_console; //Is the top bar.
        console_wnd_t main_console;
        console_wnd_t info_console;
        const sf::Font* font;
        buildings::building_manager* building_mng;
    public:
        console_manager();
        short init_consoles( const graphic_ui::game_window_config_t& window_config );
        void draw_console( sf::RenderWindow& window );
        //For writing operation
        void write_info( const std::string& msg );
        void write_status( const std::string& location );
        void handle_console_click( long x_pos , long y_pos );
        void set_building_manager( buildings::building_manager* mng );
        void enter_main_manu();
    };

}

#endif // UI_CONSOLE_HPP_INCLUDED
