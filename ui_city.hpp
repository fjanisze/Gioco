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
    enum city_ui_input_mode_t
    {
        view_mode, //This is the standard view, when the player just move around the map or menu
        building_mode //The user has clicked one build button and is willing to build something.
    };

    //Information on the field where the user want to build a construction
    struct build_trigger_info_t
    {
        long building_id; //ID of the building
        citymap::citymap_field_t* field; //Field on which the user wish to build the construction
    };

    class city_ui
    {
        cities::city_agent* city_agent;
        graphic_ui::console_manager* ui_console;
        sf::VertexArray* focus_box;
        void update_focus_box( long x_pos , long y_pos );
        long field_width,
            field_height;
        sf::RenderWindow* window;
        game_map::map_viewport_settings_t map_view_setting;
        city_ui_input_mode_t input_mode;
        build_trigger_info_t build_info;
        bool handle_new_construction();
    public:
        city_ui( sf::RenderWindow* rnd_window , game_map::map_viewport_settings_t map_setting );
        ~city_ui();
        void set_cityagent( cities::city_agent* agent );
        void set_console_manager( graphic_ui::console_manager* console );
        long handle_event( const sf::Event& event );
        void city_map_mouse_move( const sf::Event& event );
        void draw_city_ui_elements();
        void enter_city_menu();
        void mouse_press_event( const sf::Event& event );
        bool is_over_the_game_map( long x_pos , long y_pos );
        void handle_build_btn_click( long action_id );
    };
}

#endif // UI_CITY_HPP_INCLUDED
