#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "draw.hpp"
#include <GL/Gl.h>

namespace drawing_objects
{
    int drawing_context::next_context_id = 0;

    //Return a unique ID for the context
    int drawing_context::get_next_unique_id()
    {
        long cnt = 10;
        while( context_id_guard.test_and_set( std::memory_order::memory_order_acquire ) )
        {
            std::this_thread::sleep_for( std::chrono::microseconds{ 5 } );
            --cnt;
            if( cnt == 0 )
            {
                LOG_ERR("drawing_context::get_next_unique_id(): Unable to get a new unique ID, deadlock?..");
                return -1;
            }
        }
        int result = ++next_context_id;
        context_id_guard.clear();
        return result;
    }

    drawing_context::drawing_context()
    {
        context_id = get_next_unique_id();
        assert( context_id > 0 ); // :(
    }

    drawing_facility::drawing_facility() : continue_looping{ false } , draw_fps{ 35 },
                        is_render_window_ready{ false }
    {
        LOG("drawing_facility::drawing_facility(): Creating the object.");
    }

    drawing_facility::~drawing_facility()
    {
    }

    //Terminate the execution of the rendering thread and loop
    void drawing_facility::terminate()
    {
        LOG("drawing_facility::terminate(): Waiting for rendering_thread to join and then quit.");
        continue_looping = false;
        rendering_thread->join();
        LOG("drawing_facility::terminate(): Done");
    }

    //Start the drawing functionality on a separate thread,
    //start need to be followed by the terminate call
    void drawing_facility::start()
    {
        LOG("drawing_facility::start(): Starting the rendering thread.");
        rendering_thread = std::unique_ptr< std::thread >( new std::thread( &drawing_facility::rendering_loop , this ) );
    }

    //If any event in the queue, pop it
    bool drawing_facility::poll_event( sf::Event& event )
    {
        while( event_queue_guard.test_and_set() );
        bool res = false;
        if( !event_queue.empty() )
        {
            event = event_queue.front();
            event_queue.pop();
            res = true;
        }
        event_queue_guard.clear();
        return res;
    }

    //Responsible for drawing
    void drawing_facility::rendering_loop()
    {
        log_inst.set_thread_name("RENDER");

        create_render_window( ui_config );
        //Load the font
        if( !font.loadFromFile("consola.ttf") )
        {
            LOG_ERR("drawing_facility::rendering_loop(): Unable to load the selected font consola.ttf");
            std::terminate();
        }
        LOG("drawing_facility::rendering_loop(): Ready to start, fps: ", draw_fps );

        while( continue_looping )
        {
            //No adding operation allowed during the loop
            std::lock_guard< std::mutex > lock( render_mutex );
            //Clear the screen
            render_window.clear( sf::Color::Black );

            //To through all the objects in all the context
            for( auto& elem : objects )
            {
                if( elem.second->state == context_state::enabled )
                {
                    render< sf::VertexArray >( elem.second->objects.get< sf::VertexArray >() );
                    render< sf::Text >( elem.second->objects.get< sf::Text >() );
                }
            }

            render_window.display();
            //Unfortunately SFML works in a way that is possible to poll the events
            //only in the same thread which create the render window, and the render operation
            //is possible only on the same thread.
            sf::Event event;
            //Go ahead only if is possible to push new events in the queue, if not wait the next cycle
            if( !event_queue_guard.test_and_set() )
            {
                while( render_window.pollEvent( event ) )
                {
                    //Push the event in the local queue.
                    event_queue.push( event );
                }
                event_queue_guard.clear();
            }
            std::this_thread::sleep_for( std::chrono::milliseconds{ 1000 / draw_fps } );
        }
        LOG("drawing_facility::rendering_loop(): Quitting.");
    }

    //Those functions are responsible for specific procedures which cannot be handled
    //in the common to all 'render' function
    void drawing_facility::init_object( object_cnt< sf::Text >::ptr_to_obj obj )
    {
        //Set the font
        for( auto& elem : obj->get_all() )
        {
            elem->setFont( font );
        }
    }
    void drawing_facility::init_object( object_cnt< sf::VertexArray >::ptr_to_obj obj )
    {
        //Actually do nothing
    }

    drawing_facility* drawing_facility::instance = nullptr;

    drawing_facility* drawing_facility::get_instance()
    {
        if( instance == nullptr )
        {
            instance = new drawing_facility;
        }
        return instance;
    }

    //Return the render window object
    sf::RenderWindow* drawing_facility::get_RenderWindow()
    {
        //Wait until the render window is not ready to be used
        int cnt = 1;
        while( !is_render_window_ready.load( std::memory_order::memory_order_acquire ) )
        {
            //Not ready, wait for a while
            LOG_WARN("drawing_facility::get_RenderWindow(): Render window not ready, attempt #", cnt++ );
            std::this_thread::sleep_for( std::chrono::milliseconds{ 50 } );
        }
        return &render_window;
    }

    //Configure and create the appropriate window
    sf::RenderWindow& drawing_facility::create_render_window( graphic_ui::game_window_config_t& ui_config )
    {
        LOG("drawing_facility::create_render_window(): Creating the rendering window" );
        //Create the resource
        //Video mode:
        sf::VideoMode video_mode;
        video_mode.bitsPerPixel = 24;
        video_mode.height = ui_config.window_height;
        video_mode.width = ui_config.window_width;
        //Context settings:
        sf::ContextSettings context;
        context.depthBits = 24;
        context.stencilBits = 8;
        context.minorVersion = 0;
        context.majorVersion = 3;
        context.antialiasingLevel = 4;
        //Style
        UINT32 style = sf::Style::Default;
        //Full screen?
        if( ui_config.full_screen )
        {
            style |= sf::Style::Fullscreen;
        }

        render_window.create( video_mode , "Test" , style , context );
        continue_looping = true;
        //Ok, the render window can be used
        is_render_window_ready.store( true, std::memory_order::memory_order_acquire );

        return render_window;
    }

    //This render facility draw just the active rendering context
    int drawing_facility::create_render_context( const std::string& name )
    try{
        drawing_context* new_context = new drawing_context();
        new_context->name = name; //The name is optional
        objects[ new_context->context_id ] = std::unique_ptr< drawing_context >(new_context);
        LOG("drawing_facility::create_render_context(): New context ready, ID:",new_context->context_id,", name: ",name );
        return new_context->context_id;
    }catch( std::bad_alloc& xa )
    {
        LOG_ERR("drawing_facility::create_render_context(): Allocation failure..");
        return -1;
    }

    //Activate or deactivate a drawing context
    bool drawing_facility::enable_context( int num )
    {
        ELOG("drawing_facility::enable_contex(): Enabling ID:",num);
        objects[ num ]->state = context_state::enabled;
    }
    bool drawing_facility::disable_context( int num )
    {
        ELOG("drawing_facility::disable_context(): Disabling ID:",num);
        objects[ num ]->state = context_state::disabled;
    }
}
