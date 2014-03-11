#ifndef UI_COMMON_CPP_INCLUDED
#define UI_COMMON_CPP_INCLUDED

#include "ui_common.hpp"

namespace graphic_ui
{
    game_window_config_t::game_window_config_t()
    {
        //Default setting
        full_screen = false;
        //Window size in pixels.
        window_height = 650;
        window_width = 800;
        //Game viewport
        map_canvas_setting.canvas_x_offset = 0;
        map_canvas_setting.canvas_y_offset = 20;
        map_canvas_setting.canvas_width = 600;
        map_canvas_setting.canvas_height = 600;
    }
}

#endif // UI_COMMON_CPP_INCLUDED
