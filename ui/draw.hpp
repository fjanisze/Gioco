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
#include <list>

#ifndef DRAW_HPP
#define DRAW_HPP


namespace drawing_objects
{
    class drawing_facility;

    typedef enum draw_type {
        draw_text = 0,
        draw_vertex = 1
    } draw_type;

    typedef enum draw_obj_render_state {
        render = 0,
        do_not_render //Skip the rendering of the object
    } draw_obj_render_state;

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
        std::atomic_flag render_init_done{ ATOMIC_FLAG_INIT };
    private:
        drawable_obj_cnt objects; //The drawable object container
        draw_type object_type;
        draw_obj_render_state render_state{ draw_obj_render_state::render };
    public:
        //static drawable_object* create( draw_type type , sf::Drawable* first_obj = nullptr );
        drawable_object();
        drawable_object( OBJ_T* first_obj );
        drawable_object( const drawable_object& other ) = delete;
        drawable_object& operator=( drawable_object&& other ) = delete;
        long clear();
        //update functions, replace the existing information with the new one
        long update( std::vector< OBJ_T* >& new_obj );
        long update( OBJ_T* new_obj );
        //Append new objects
        long add( std::vector< OBJ_T* >& new_obj );

        //Get the proper object
        OBJ_T& get() throw( std::range_error );
        OBJ_T& get( int pos ) throw( std::range_error );
        drawable_obj_cnt& get_all();

        void lock();
        void unlock();
        draw_obj_render_state get_render_state();
        draw_obj_render_state set_render_state( draw_obj_render_state new_state );
    };

    template< typename OBJ_T >
    void drawable_object< OBJ_T >::lock()
    {
        mutex.lock();
    }

    template< typename OBJ_T >
    void drawable_object< OBJ_T >::unlock()
    {
        glFlush(); //Force the context update
        mutex.unlock();
    }

    template< typename OBJ_T >
    draw_obj_render_state drawable_object< OBJ_T >::get_render_state()
    {
        return render_state;
    }

    template< typename OBJ_T >
    draw_obj_render_state drawable_object< OBJ_T >::set_render_state( draw_obj_render_state new_state )
    {
        draw_obj_render_state old_state = render_state;
        render_state = new_state;
        return old_state;
    }

    //Get the proper object, the object can be modified.
    template< typename OBJ_T >
    OBJ_T& drawable_object< OBJ_T >::get() throw( std::range_error )
    {
        if( objects.empty() ) throw std::range_error("Objects is empty!");
        render_init_done.clear();
        return *objects[ 0 ];
    }

    template< typename OBJ_T >
    OBJ_T& drawable_object< OBJ_T >::get( int pos ) throw( std::range_error )
    {
        if( pos < 0 || pos > objects.size() ) throw std::range_error("Not valid index");
        render_init_done.clear();
        return *objects[ pos ];
    }

    //Update the elements in the drawable object container
    template< typename OBJ_T >
    long drawable_object< OBJ_T >::update( std::vector< OBJ_T* >& new_obj )
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
        render_init_done.clear();
        glFlush();
        return objects.size();
    }
    template< typename OBJ_T >
    long drawable_object< OBJ_T >::update( OBJ_T* new_obj )
    {
        std::lock_guard< std::mutex > lock( mutex );
        objects.clear();
        std::unique_ptr< OBJ_T > obj_cp( new_obj );
        //Copy the new elements
        objects.push_back( std::move( obj_cp ) );
        render_init_done.clear();
        glFlush();
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
        render_init_done.clear();
        glFlush();
        return objects.size();
    }

    //Default empty constructor
    template< typename OBJ_T >
    drawable_object< OBJ_T >::drawable_object()
    {
    }

    //Create a drawable object with one element
    template< typename OBJ_T >
    drawable_object< OBJ_T >::drawable_object( OBJ_T* first_obj )
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
        render_init_done.clear();
        return sz;
    }

    //Return a reference to the whole container
    template< typename OBJ_T >
    typename drawable_object< OBJ_T >::drawable_obj_cnt& drawable_object< OBJ_T >::get_all()
    {
        return objects;
    }

    //Define the types for the container
    template< typename T >
    struct object_cnt
    {
        typedef std::shared_ptr< drawable_object< T > > ptr_to_obj;
        typedef std::vector< ptr_to_obj > type;
    };


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
        typename object_cnt< OBJ >::type& get()
        {
            return dynamic_cast< box< drawable_object< OBJ > >* >( this )->obj;
        }
    };

    //Possile context states
    typedef enum context_state {
        enabled = 0,
        disabled,
        not_ready
    } context_state;

    //Drawing context information
    struct drawing_context
    {
        int context_id;
        context_state state{ context_state::not_ready };
        static int next_context_id;
        std::atomic_flag context_id_guard{ ATOMIC_FLAG_INIT };
        std::string name;

        drawing_context();
        //Container for all the drawable objects
        drawable_obj_container< sf::Text , sf::VertexArray > objects;
        //Functionalities needed to create the context id
        int get_next_unique_id();
    };

    //The draw facility is responsible for drawing the drawable objects on the screen
    class drawing_facility
    {
        static drawing_facility* instance;
        std::mutex render_mutex{};
        //std::map< int , drawable_obj_container< sf::Text , sf::VertexArray > > objects;
        std::map< int , std::unique_ptr< drawing_context > > objects;
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
    private:
        void init_object( object_cnt< sf::Text >::ptr_to_obj obj );
        void init_object( object_cnt< sf::VertexArray >::ptr_to_obj obj );
        template< typename T >
        long render( typename object_cnt< T >::type& elements );
    public:
        static drawing_facility* get_instance();
        drawing_facility();
        ~drawing_facility();
        template< typename OBJ_T >
        long add( drawable_object< OBJ_T >* obj_ptr , int context_id );
        void start();
        void terminate();
        sf::RenderWindow* get_RenderWindow();
        bool poll_event( sf::Event& event );
        int create_render_context( const std::string& name = "NA" );
        bool enable_context( int num );
        bool disable_context( int num );
    };

    //Add the drawable object to the internal container
    template< typename OBJ_T >
    long drawing_facility::add( drawable_object< OBJ_T >* obj_ptr, int context_id )
    {
        std::lock_guard< std::mutex > lock( render_mutex );
        //Check if the context exist
        if( objects.find( context_id ) == objects.end() )
        {
            LOG_ERR("drawing_facility::add(): Unable to find the context with ID:", context_id );
            return -1;
        }
        typename object_cnt< OBJ_T >::type& container = objects[ context_id ]->objects.get< OBJ_T >();
        container.push_back( std::shared_ptr< drawable_object< OBJ_T > >( obj_ptr ) );
        return container.size();
    }


    //Perform the rendering
    template< typename T >
    long drawing_facility::render( typename object_cnt< T >::type& elements )
    {
        for( auto obj : elements )
        {
            //Should the object be rendered?
            if( obj->render_state != draw_obj_render_state::render )
            {
                //skip
                continue;
            }
            //Can i used the object?
            if( !obj->mutex.try_lock() )
            {
                continue; //Just skip the not usable items -locked-
            }
            //Any computation needed?
            if( !obj->render_init_done.test_and_set( std::memory_order::memory_order_acquire ) )
            {
                init_object( obj );
            }
            //Ok
            for( auto& elem : obj->get_all() )
            {
                //Draw the object
                render_window.draw( *elem );
            }
            obj->mutex.unlock();
        }
    }
}


#endif
