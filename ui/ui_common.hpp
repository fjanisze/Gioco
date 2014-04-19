#ifndef UI_COMMON_HPP_INCLUDED
#define UI_COMMON_HPP_INCLUDED

#include "../map/map_common.hpp"
#include <memory>

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
        game_map::game_canvas_settings_t map_canvas_setting;
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

    struct size_and_pos_t
    {
        int id;
        long x,
             y;
        long width,
             height;
    };

    //Position and size of a console
    struct console_pos_and_size_t
    {
        int x_pos,
            y_pos;
        int width,
            height;
        //Button grid, not necessary exist
        int num_of_button;
        std::unique_ptr<size_and_pos_t> button_grid;
    };
}

#endif // UI_COMMON_HPP_INCLUDED
