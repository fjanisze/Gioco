#ifndef UI_COMMON_HPP_INCLUDED
#define UI_COMMON_HPP_INCLUDED

#include "../map/map_common.hpp"

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

    //Possible type of view
    enum type_of_view_t
    {
        game_map_view,
        city_map_view
    };
}

#endif // UI_COMMON_HPP_INCLUDED
