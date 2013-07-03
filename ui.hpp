#include "logging/logger.hpp"
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <cassert>
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

        game_map::map_viewport_settings_t viewport_setting;

        game_window_config_t();
    };

    //Manage the whole ui
    class game_ui
    {
        static game_ui* instance;
        game_map::game_map* map;
    private:
        game_window_config_t ui_config;
        bool is_the_window_running;
        sf::RenderWindow window;
        sf::RenderWindow& create_render_window();
        void handle_event( const sf::Event& event );
        void screen_refresh();
        void draw_gameplay_map();
    public:
        static game_ui* get_instance();
        void main_loop();
        game_ui();
        ~game_ui();
    };
}
