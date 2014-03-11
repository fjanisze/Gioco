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
        void add_to_graphic_context( long contex_id );
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

    //Available states
    typedef enum available_state_ids {
        map_view_default = 0,
        city_view_default,
        city_view_build_default,
    } available_state_ids;


    //This object is reponsible for the console management
    class console_manager
    {
    protected:
        type_of_view_t current_view; //On the base of the current view different action are possible
        std::mutex mutex;
    private:
        bool handle_non_common_button( graphic_elements::ui_button_t* button  );
    public:
        console_manager();
        //For writing operation
        void write_info( const std::string& msg );
        void write_status( const std::string& location );
        void handle_console_click( long x_pos , long y_pos );
        void show_city_main_menu();
        void show_map_main_menu();
        void button_trigger_action( graphic_elements::ui_button_t* button );
    };

}

#endif // UI_CONSOLE_HPP_INCLUDED
