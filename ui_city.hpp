#ifndef UI_CITY_HPP_INCLUDED
#define UI_CITY_HPP_INCLUDED

#include "logging/logger.hpp"
#include "map_common.hpp"
#include "ui_console.hpp"

/*
 * Here the code which manage the city UI is present, each time we are in type_of_view_t::city_map_view the game will make use of those functions
 */
namespace city_ui_manager
{
    class city_ui
    {
        citymap::city_agent* city_agent;
        graphic_ui::console_manager* ui_console;
    public:
        city_ui();
        void set_cityagent( citymap::city_agent* agent );
        void set_console_manager( graphic_ui::console_manager* console );
        long handle_event( const sf::Event& event );
        void city_map_mouse_move( const sf::Event& event );
    };
}

#endif // UI_CITY_HPP_INCLUDED
