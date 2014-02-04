#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#ifndef UI_CONSOLE_HPP_INCLUDED
#define UI_CONSOLE_HPP_INCLUDED

#include "../logging/logger.hpp"
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "ui_common.hpp"
#include "../buildings.hpp"
#include "draw.hpp"
#include <sstream>
#include <string>
#include <map>
#include <mutex>
#include <chrono>
#include <thread>

#define COMMON_BUILD_BUTTON 0
#define COMMON_MAP_BUTTON 1
#define COMMON_QUIT_BUTTON 2 //Quit the application

#define BUILDING_BUTTON_ID_BEGIN 1000
#define BUILDING_BUTTON_ID_END 1999

namespace graphic_elements
{
    //Button entity, the user can click to trigger some action
    class ui_button_t
    {
        int button_context_id;
        drawing_objects::drawable_object< sf::VertexArray > vertex;
        long reference_id;
        drawing_objects::drawable_object< sf::Text > button_text;
        long action_id; //A kind of code which can be used to identify which action need to be triggered.
    public:
        ui_button_t();
        ~ui_button_t();
        ui_button_t( ui_button_t&& other ) = delete;
        ui_button_t& operator=( const ui_button_t& other ) = delete;
        ui_button_t& operator=( ui_button_t&& other ) = delete;
        void create( long x_pos, long y_pos, long width, long height );
        void set_text( const std::string& text );
        sf::Text get_text();
        void set_appearence( const sf::Color& color );
        void set_offset( long x_axis, long y_axis );
        void set_id( long id );
        long get_id() const;
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
        std::shared_ptr< graphic_elements::ui_button_t > button;
        console_point_t() : is_console_point( false ) ,
                            x_pos( -1 ) , y_pos( -1 ),
                            click_possible( false ),
                            action_id( -1 )
        {
        }
    };

    //What is visible on the console?
    class console_wnd_state
    {
        int state_graphic_context;
        std::vector< long > visible_buttons;
        long console_id;
    public:
        console_wnd_state( long origin_console_id );
    };

    //Console Window information
    class console_wnd_t
    {
        //Unique ID for the console
        static long next_console_wnd_id;
        long get_next_id();
        long console_wnd_id;
        std::string console_name;
        //Those values specify where the top left corner of the console is places
        long x_offset,
            y_offset;
        //Size
        long width,
            height;
        int console_graphic_context_id;
        //Related vertex
        drawing_objects::drawable_object< sf::VertexArray > background;
        //Position of all the buttons
        std::vector< button_position_t > button_map;
        //Text visible on the console
        drawing_objects::drawable_object< sf::Text > text;
        std::map< long , std::shared_ptr< graphic_elements::ui_button_t > > buttons; //Button on the console
    public:
        //Constructor and utility
        console_wnd_t( long x_off , long y_off , long wnd_width, long wnd_height );
        console_wnd_t();
        int create( long x_off , long y_off , long wnd_width, long wnd_height );
        void set_color( sf::Color color );
        void set_name( const std::string& name );
        std::string get_text();
        void set_text( const std::string& msg );
        console_point_t over_the_console( long x , long y);
        void add_button_map( const button_position_t* map , short amount_of_buttons );
        int add_button( std::shared_ptr< graphic_elements::ui_button_t >& button , short index );
        void remove_all_buttons();
        int get_render_contex_id();
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
        bool handle_non_common_button( graphic_elements::ui_button_t* button  );
    public:
        console_manager();
        std::vector< console_wnd_t* > init_consoles( const graphic_ui::game_window_config_t& window_config );
        //For writing operation
        void write_info( const std::string& msg );
        void write_status( const std::string& location );
        void handle_console_click( long x_pos , long y_pos );
        short add_building_construction_btn( console_wnd_t& console , long y_pos = 0  );
        long add_city_common_btn( console_wnd_t& console );
        long add_map_common_btn( console_wnd_t& console );
        void show_city_main_menu();
        void show_map_main_menu();
        void button_trigger_action( graphic_elements::ui_button_t* button );
    };

}

#endif // UI_CONSOLE_HPP_INCLUDED
