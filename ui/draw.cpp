#define LOGGING_LEVEL_1
#define LOGGING_LEVEL_2

#include "draw.hpp"

namespace drawing_objects
{
    //Create one of the possible objects
 /*   drawable_object* drawable_object_trait::create( draw_type type , sf::Drawable* first_obj  )
    try{
        drawable_object* object = new drawable_object;
        object->object_type = type;
        if( first_obj != nullptr )
        {
            //update( first_obj );
        }
        return object;
    }catch( std::bad_alloc& xa )
    {
        LOG_ERR("drawable_object_trait::create(): Unable to allocate the memory for the required object..");
        return nullptr;
    }*/


    drawing_facility::drawing_facility() : continue_looping{ false } , draw_fps{ 30 },
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
            std::lock_guard< std::mutex > lock( write_mutex );
            //To through all the objects
            render( objects.get< sf::Text >() );
            render( objects.get< sf::VertexArray >() );

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

        render_window.create( video_mode , "Economics" , style , context );
        continue_looping = true;
        //Ok, the render window can be used
        is_render_window_ready.store( true, std::memory_order::memory_order_acquire );

        return render_window;
    }
}
