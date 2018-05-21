#ifndef __WFN_ENG_SDL_HPP__
#define __WFN_ENG_SDL_HPP__

#include <string>
#include <SDL.h>

namespace wfn_eng::sdl {
    ////
    // struct WindowConfig
    //
    // A container for several configuration flags that can be passed to the
    // Window class to define its construction.
    struct WindowConfig {
        std::string vulkanPath;
        std::string windowName;
        int width;
        int height;
        uint32_t flags;
    };

    ////
    // class Window
    //
    // A wrapper around the (SDL_Window *) object to provide construction and
    // destruction capabilities.
    class Window {
        SDL_Window *window;

    public:
        ////
        // Window
        //
        // Given a WindowConfig (defined above), construct a Window.
        Window(WindowConfig);

        ////
        // ~Window()
        //
        // Custom destructor handles the cleanup up the SDL_Window *.
        ~Window();

        ////
        // SDL_Window *ref()
        //
        // Returns a reference to the internal SDL window attached to this
        // object.
        SDL_Window *ref();

        // Rule of threes
        Window(const Window&) = delete;
        Window& operator=(const Window&) = delete;
    };
}

#endif
