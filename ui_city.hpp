#ifndef UI_CITY_HPP_INCLUDED
#define UI_CITY_HPP_INCLUDED

#include "logging/logger.hpp"
#include "map_common.hpp"
#include "ui_console.hpp"
#include "city_map.hpp"
#include <cassert>

/*
 * Here the code which manage the city UI is present, each time we are in type_of_view_t::city_map_view the game will make use of those functions
 */
namespace city_ui_manager
{
    class city_ui
    {
        citymap::city_agent* city_agent;
        graphic_ui::console_manager* ui_console;
        sf::VertexArray* focus_box;
        void update_focus_box( long x_pos , long y_pos );
        long field_width,
            field_height;
        sf::RenderWindow* window;
        game_map::map_viewport_settings_t map_view_setting;
    public:
        city_ui( sf::RenderWindow* rnd_window , game_map::map_viewport_settings_t map_setting );
        ~city_ui();
        void set_cityagent( citymap::city_agent* agent );
        void set_console_manager( graphic_ui::console_manager* console );
        long handle_event( const sf::Event& event );
        void city_map_mouse_move( const sf::Event& event );
        void draw_city_ui_elements();
    };
}

#endif // UI_CITY_HPP_INCLUDED
