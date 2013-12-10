#include "../logging/logger.hpp"
#include "ui_common.hpp"
#include <SFML/OpenGL.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <iterator>
#include <vector>
#include <mutex>
#include <typeinfo>
#include <memory>
#include <chrono>
#include <map>

#include <iostream>

#ifndef DRAW_HPP
#define DRAW_HPP

typedef enum use_font {
    yes,
    no
} use_font;

namespace drawing_objects
{
    class drawing_facility;

    using drawable_obj_cnt = std::vector< std::unique_ptr< sf::Drawable > >;

    typedef enum draw_type {
        draw_text,
        draw_vertex
    } draw_type;

    //This object contains graphic information that can be used to
    //draw something on the screen. Actually, is just a kind of wrapper
    //for Vertex, VertexArray and sf::Text from SFML
    //template< OBJ_T >
    class drawable_object
    {
        friend class drawing_facility;
        friend class drawable_object_factory;
        std::mutex mutex{};
    private:
        drawable_obj_cnt objects; //The drawable object container
        bool use_font_when_rendering{ false };
        draw_type object_type;
    public:
        //static drawable_object* create( draw_type type , sf::Drawable* first_obj = nullptr );
        drawable_object();
        drawable_object( sf::Drawable* first_obj , use_font apply_font = use_font::no );
        drawable_object( const drawable_object& other ) = delete;
        drawable_object& operator=( drawable_object&& other ) = delete;
        long clear();
        //update functions, replace the existing information with the new one
        template< typename THIS_OBJ_T >
        long update( std::vector< THIS_OBJ_T* >& new_obj , use_font apply_font = use_font::no );
        template< typename THIS_OBJ_T >
        long update( THIS_OBJ_T* new_obj , use_font apply_font = use_font::no );
        //Append new objects
        template< typename THIS_OBJ_T >
        long add( std::vector< THIS_OBJ_T* >& new_obj );

        //Get the proper object
        template< typename THIS_OBJ_T >
        THIS_OBJ_T& get() throw( std::range_error );
        template< typename THIS_OBJ_T >
        THIS_OBJ_T& get( int pos ) throw( std::range_error );
        drawable_obj_cnt& get_all();

        void lock()
        {
            mutex.lock();
        }
        void unlock()
        {
            mutex.unlock();
        }
        bool apply_font()
        {
            return use_font_when_rendering;
        }
    };

    //Get the proper object, the object can be modified.
    template< typename THIS_OBJ_T >
    THIS_OBJ_T& drawable_object::get() throw( std::range_error )
    {
        if( objects.empty() ) throw std::range_error("Objects is empty!");
        return dynamic_cast< THIS_OBJ_T& >( *objects[ 0 ] );
    }

    template< typename THIS_OBJ_T >
    THIS_OBJ_T& drawable_object::get( int pos ) throw( std::range_error )
    {
        if( pos < 0 || pos > objects.size() ) throw std::range_error("Not valid index");
        return dynamic_cast< THIS_OBJ_T& >( *objects[ pos ] );
    }

    //Update the elements in the drawable object container
    template< class THIS_OBJ_T >
    long drawable_object::update( std::vector< THIS_OBJ_T* >& new_obj , use_font apply_font )
    {
        std::lock_guard< std::mutex > lock( mutex );
        objects.clear();
        //Make a copy of the objects
        auto make_copy_and_push_back = [ this ]( sf::Drawable* obj ) {
            std::unique_ptr< sf::Drawable > obj_cp( obj );
            objects.push_back( std::move( obj_cp ) ); //push
        };
        //Copy the new elements
        std::for_each( std::begin( new_obj ) , std::end( new_obj ) , make_copy_and_push_back );
        use_font_when_rendering = ( apply_font == use_font::yes );
        return objects.size();
    }
    template< class THIS_OBJ_T >
    long drawable_object::update( THIS_OBJ_T* new_obj, use_font apply_font )
    {
        std::lock_guard< std::mutex > lock( mutex );
        objects.clear();
        std::unique_ptr< sf::Drawable > obj_cp( new_obj );
        //Copy the new elements
        objects.push_back( std::move( obj_cp ) );
        use_font_when_rendering = ( apply_font == use_font::yes );
        return objects.size();
    }

    //Add the new objects
    template< class THIS_OBJ_T >
    long drawable_object::add( std::vector< THIS_OBJ_T* >& new_obj )
    {
        std::lock_guard< std::mutex > lock( mutex );
        //Make a copy of the objects
        auto make_copy_and_push_back = [ this ]( sf::Drawable* obj ) {
            std::unique_ptr< sf::Drawable > obj_cp( obj );
            objects.push_back( std::move( obj_cp ) ); //push
        };
        std::for_each( std::begin( new_obj ) , std::end( new_obj ), make_copy_and_push_back );
        return objects.size();
    }

    //The draw facility is responsible for drawing the drawable objects on the screen
    class drawing_facility
    {
        static drawing_facility* instance;
        std::mutex write_mutex{};
        std::vector< drawable_object* > objects;
        //std::map< draw_type , std::vector< std::shared_ptr< drawable_object >>> objects;
        std::atomic_flag event_queue_guard{ ATOMIC_FLAG_INIT };
        std::queue< sf::Event > event_queue;
        bool continue_looping;
        int draw_fps;
        sf::RenderWindow render_window;
        sf::Font font;
        std::atomic_bool is_render_window_ready; //By default is false
        graphic_ui::game_window_config_t ui_config;
        std::unique_ptr< std::thread > rendering_thread;
        void rendering_loop();
        sf::RenderWindow& create_render_window( graphic_ui::game_window_config_t& ui_config );
    public:
        static drawing_facility* get_instance();
        drawing_facility();
        ~drawing_facility();
        void start();
        void terminate();
        long add( drawable_object* obj_ptr );
        sf::RenderWindow* get_RenderWindow();
        bool poll_event( sf::Event& event );
    };
}


#endif
