#ifndef UI_HPP
#define UI_HPP

#include "../logging/logger.hpp"
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <cassert>
#include <iostream>
#include "../map/map.hpp"
#include "../map/city_map.hpp"
#include "draw.hpp"
#include "ui_city.hpp"
#include "ui_console.hpp"
#include "ui_common.hpp"
#include <thread>

namespace graphic_ui
{
    //Manage the whole ui
    class game_ui : public console_manager
    {
        static game_ui* instance;
        game_map::game_map* map;
        cities::city_agent* current_city;
        city_ui_manager::city_ui* city_ui;
        drawing_objects::drawing_facility* draw_facility;
    public:
        bool is_over_the_game_map( long x_pos , long y_pos );
    private:
        void load_and_set_font();
        game_window_config_t ui_config;
        bool is_the_window_running;
        sf::RenderWindow* window;
        void handle_event( const sf::Event& event );
        void mouse_moving_event( const sf::Event& event );
        void mouse_press_event( const sf::Event& event );
        void game_map_mouse_move( const sf::Event& event );
        void city_map_mouse_move( const sf::Event& event );
        void print_status_info();
    public:
        void set_view_to_citymap();
        void set_view_to_gamemap();
        static game_ui* get_instance();
        city_ui_manager::city_ui* get_city_ui();
        void main_loop();
        void quit();
        game_map::game_canvas_settings_t get_game_canvas_settings();
        game_ui();
        ~game_ui();
    };
}

#endif
