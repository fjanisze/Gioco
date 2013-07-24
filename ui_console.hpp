#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#ifndef UI_CONSOLE_HPP_INCLUDED
#define UI_CONSOLE_HPP_INCLUDED

#include "logging/logger.hpp"
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include "ui_common.hpp"

namespace graphic_ui
{
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
        console_wnd_t status_console; //Is the top bar.
        console_wnd_t main_console;
        console_wnd_t info_console;
        const sf::Font* font;
    public:
        console_manager();
        short init_consoles( const graphic_ui::game_window_config_t& window_config );
        void draw_console( sf::RenderWindow& window );
        //For writing operation
        void write_info( const std::string& msg );
    };

}

#endif // UI_CONSOLE_HPP_INCLUDED
