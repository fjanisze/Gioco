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

#define COMMON_BUILD_BUTTON 0
#define COMMON_MAP_BUTTON 1

#define BUILDING_BUTTON_ID_BEGIN 1000
#define BUILDING_BUTTON_ID_END 1999

namespace graphic_elements
{
    //Button entity, the user can click to trigger some action
    class ui_button_t
    {
        sf::VertexArray vertex; //Graphic appearence
        long reference_id;
        sf::Text button_text;
        long action_id; //A kind of code which can be used to identify which action need to be triggered.
    public:
        ui_button_t();
        void create( long x_pos, long y_pos, long width, long height );
        void set_text( const std::string& text , const sf::Font* font );
        sf::Text get_text();
        void set_appearence( const sf::Color& color );
        sf::VertexArray& get_vertex();
        void set_offset( long x_axis, long y_axis );
        void set_id( long id );
        long get_id();
        void set_action_id( long id );
        long get_action_id();
        bool is_point_over_the_button( long x_pos , long y_pos );
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
        long action_id; //Id related with the action triggered by the click, for simplicity reason this is the button id
        graphic_elements::ui_button_t* button;
        console_point_t() : is_console_point( false ) ,
                            x_pos( -1 ) , y_pos( -1 ),
                            click_possible( false ),
                            action_id( -1 ),
                            button( nullptr )
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
        void set_font( const sf::Font* fnt  );
        std::vector< sf::VertexArray >& get_vertex();
        sf::VertexArray& get_background_vertex();
        sf::Text& get_text();
        void add_button_map( const button_position_t* map , short amount_of_buttons );
        void add_button( graphic_elements::ui_button_t button , short index );
        void draw( sf::RenderWindow& window );
        void remove_all_buttons();
    };

    //This object is reponsible for the console management
    class console_manager
    {
    protected:
        type_of_view_t current_view; //On the base of the current view different action are possible
        std::mutex mutex;
    private:
        //We assume that only two console are provided at the beginning
        console_wnd_t status_console; //Is the top bar.
        console_wnd_t main_console;
        console_wnd_t info_console;
        const sf::Font* font;
        bool handle_non_common_button( graphic_elements::ui_button_t* button  );
    public:
        console_manager();
        short init_consoles( const graphic_ui::game_window_config_t& window_config );
        void draw_console( sf::RenderWindow& window );
        //For writing operation
        void write_info( const std::string& msg );
        void write_status( const std::string& location );
        void handle_console_click( long x_pos , long y_pos );
        void add_building_construction_btn( console_wnd_t& console , long y_pos = 0  );
        long add_city_common_btn( console_wnd_t& console );
        long add_map_common_btn( console_wnd_t& console );
        void show_city_main_menu();
        void show_map_main_menu();
        void button_trigger_action( graphic_elements::ui_button_t* button );
    };

}

#endif // UI_CONSOLE_HPP_INCLUDED
