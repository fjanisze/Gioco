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
        draw_text = 0,
        draw_vertex = 1
    } draw_type;

    //This object contains graphic information that can be used to
    //draw something on the screen. Actually, is just a kind of wrapper
    //for Vertex, VertexArray and sf::Text from SFML
    template< typename OBJ_T >
    class drawable_object
    {
    public:
        using drawable_obj_cnt = std::vector< std::unique_ptr< OBJ_T > >;
    private:
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
        drawable_object( OBJ_T* first_obj , use_font apply_font = use_font::no );
        drawable_object( const drawable_object& other ) = delete;
        drawable_object& operator=( drawable_object&& other ) = delete;
        long clear();
        //update functions, replace the existing information with the new one
        long update( std::vector< OBJ_T* >& new_obj , use_font apply_font = use_font::no );
        long update( OBJ_T* new_obj , use_font apply_font = use_font::no );
        //Append new objects
        long add( std::vector< OBJ_T* >& new_obj );

        //Get the proper object
        OBJ_T& get() throw( std::range_error );
        OBJ_T& get( int pos ) throw( std::range_error );
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
    template< typename OBJ_T >
    OBJ_T& drawable_object< OBJ_T >::get() throw( std::range_error )
    {
        if( objects.empty() ) throw std::range_error("Objects is empty!");
        return *objects[ 0 ];
    }

    template< typename OBJ_T >
    OBJ_T& drawable_object< OBJ_T >::get( int pos ) throw( std::range_error )
    {
        if( pos < 0 || pos > objects.size() ) throw std::range_error("Not valid index");
        return *objects[ pos ];
    }

    //Update the elements in the drawable object container
    template< typename OBJ_T >
    long drawable_object< OBJ_T >::update( std::vector< OBJ_T* >& new_obj , use_font apply_font )
    {
        std::lock_guard< std::mutex > lock( mutex );
        objects.clear();
        //Make a copy of the objects
        auto make_copy_and_push_back = [ this ]( OBJ_T* obj ) {
            std::unique_ptr< OBJ_T > obj_cp( obj );
            objects.push_back( std::move( obj_cp ) ); //push
        };
        //Copy the new elements
        std::for_each( std::begin( new_obj ) , std::end( new_obj ) , make_copy_and_push_back );
        use_font_when_rendering = ( apply_font == use_font::yes );
        return objects.size();
    }
    template< typename OBJ_T >
    long drawable_object< OBJ_T >::update( OBJ_T* new_obj, use_font apply_font )
    {
        std::lock_guard< std::mutex > lock( mutex );
        objects.clear();
        std::unique_ptr< OBJ_T > obj_cp( new_obj );
        //Copy the new elements
        objects.push_back( std::move( obj_cp ) );
        use_font_when_rendering = ( apply_font == use_font::yes );
        return objects.size();
    }

    //Add the new objects
    template< typename OBJ_T >
    long drawable_object< OBJ_T >::add( std::vector< OBJ_T* >& new_obj )
    {
        std::lock_guard< std::mutex > lock( mutex );
        //Make a copy of the objects
        auto make_copy_and_push_back = [ this ]( OBJ_T* obj ) {
            std::unique_ptr< OBJ_T > obj_cp( obj );
            objects.push_back( std::move( obj_cp ) ); //push
        };
        std::for_each( std::begin( new_obj ) , std::end( new_obj ), make_copy_and_push_back );
        return objects.size();
    }

    //Default empty constructor
    template< typename OBJ_T >
    drawable_object< OBJ_T >::drawable_object()
    {
    }

    //Create a drawable object with one element
    template< typename OBJ_T >
    drawable_object< OBJ_T >::drawable_object( OBJ_T* first_obj , use_font apply_font ) :
            use_font_when_rendering{ apply_font == use_font::yes }
    {
        std::unique_ptr< OBJ_T > obj( first_obj );
        objects.push_back( std::move( obj ) );
    }

    //Clear the content for the drawable object
    template< typename OBJ_T >
    long drawable_object< OBJ_T >::clear()
    {
        std::lock_guard< std::mutex > lock( mutex );
        long sz = objects.size();
        objects.clear();
        return sz;
    }

    //Return a reference to the whole container
    template< typename OBJ_T >
    typename drawable_object< OBJ_T >::drawable_obj_cnt& drawable_object< OBJ_T >::get_all()
    {
        return objects;
    }

    //drawable_object container, very basic implementation of a tuple
    template< typename T >
    struct box
    {
        std::vector< std::shared_ptr< T > > obj;
    };
    template< typename...ARGV >
    class drawable_obj_container : public box< drawable_object< ARGV > >...
    {
    public:
        template< typename OBJ >
        std::vector< std::shared_ptr< drawable_object< OBJ > > >& get()
        {
            return dynamic_cast< box< drawable_object< OBJ > >* >( this )->obj;
        }
    };

    //The draw facility is responsible for drawing the drawable objects on the screen
    class drawing_facility
    {
        static drawing_facility* instance;
        std::mutex write_mutex{};
        //std::vector< drawable_object* > objects;
        drawable_obj_container< sf::Text , sf::VertexArray > objects;
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
        template< typename T >
        long render( std::vector< std::shared_ptr< drawable_object< T > > >& elements );
    public:
        static drawing_facility* get_instance();
        drawing_facility();
        ~drawing_facility();
        template< typename OBJ_T >
        long add( drawable_object< OBJ_T >* obj_ptr );
        void start();
        void terminate();
        sf::RenderWindow* get_RenderWindow();
        bool poll_event( sf::Event& event );
    };

    //Add the drawable object to the internal container
    template< typename OBJ_T >
    long drawing_facility::add( drawable_object< OBJ_T >* obj_ptr )
    {
        ELOG("drawing_facility::add(): Adding a new object.");
        std::lock_guard< std::mutex > lock( write_mutex );
        objects.get< OBJ_T >().push_back( std::shared_ptr< drawable_object< OBJ_T > >( obj_ptr ) );
        return objects.get< OBJ_T >().size();
    }

    //Perform the rendering
    template< typename T >
    long drawing_facility::render( std::vector< std::shared_ptr< drawable_object< T > > >& elements )
    {
        for( auto obj : elements )
        {
            //Can i used the object?
            if( !obj->mutex.try_lock() )
            {
                continue; //Just skip the not usable items -locked-
            }
            //Ok
            for( auto& elem : obj->get_all() )
            {
                render_window.draw( *elem );
            }
            obj->mutex.unlock();
        }
    }
}


#endif
