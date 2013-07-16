#ifndef UI_HPP
#define UI_HPP

#include "logging/logger.hpp"
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <cassert>
#include <iostream>
#include "map.hpp"

namespace graphic_ui
{
    //Structure which contain the setting for the UI
    struct game_window_config_t
    {
        //If true, the the game runs on full screen
        bool full_screen;
        //Size of the whole game window.
        long window_width,
            window_height;

        //Viewport configuration
        game_map::map_viewport_settings_t viewport_setting;
        //Font and text stuff
        sf::Font font;

        game_window_config_t();
    };

    //Console Window information
    struct console_wnd_t
    {
        //Those values specify where the top left corner of the console is places
        long x_offset,
            y_offset;
        //Size
        long width,
            height;
        //Related vertex, to draw a background or something
        sf::VertexArray vertex;
        //Text entity related with this console
        sf::Text text;

        //Constructor and utility
        console_wnd_t( long x_off , long y_off , long wnd_width, long wnd_height );
        console_wnd_t();
        void create( long x_off , long y_off , long wnd_width, long wnd_height );
        void set_color( sf::Color color );
    };

    //This object is reponsible for the console management
    class console_manager
    {
        //We assume that only two console are provided at the beginning
        console_wnd_t main_console;
        console_wnd_t info_console;
        const sf::Font* font;
    public:
        console_manager();
        short init_consoles( const game_window_config_t& window_config );
        void draw_console( sf::RenderWindow& window );
        //For writing operation
        void write_info( const std::string& msg );
    };

    //Possible type of view
    enum type_of_view_t
    {
        game_map_view,
        city_map_view
    };

    //Manage the whole ui
    class game_ui : public console_manager
    {
        static game_ui* instance;
        game_map::game_map* map;
        type_of_view_t current_view; //On the base of the current view different action are possible
        citymap::city_agent* current_city;
    public:
        bool is_over_the_game_map( const sf::Event& event );
    private:
        void load_and_set_font();
        game_window_config_t ui_config;
        bool is_the_window_running;
        sf::RenderWindow window;
        void handle_event( const sf::Event& event );
        void screen_refresh();
        void draw_gameplay_map();
        void draw_current_city();
        void mouse_moving_event( const sf::Event& event );
        void mouse_press_event( const sf::Event& event );
        void game_map_mouse_move( const sf::Event& event );
    public:
        sf::RenderWindow& create_render_window();
        static game_ui* get_instance();
        void main_loop();
        game_map::map_viewport_settings_t get_viewport_settings();
        game_ui();
        ~game_ui();
    };
}

#endif
