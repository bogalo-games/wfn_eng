#include "../sdl.hpp"

#include <SDL_vulkan.h>

namespace wfn_eng::sdl {
    ////
    // Window
    //
    // Given a WindowConfig (defined above), construct a Window.
    Window::Window(WindowConfig cfg) {
        SDL_Init(SDL_INIT_EVERYTHING);
        SDL_Vulkan_LoadLibrary(cfg.vulkanPath.c_str());
        window = SDL_CreateWindow(
            cfg.windowName.c_str(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            cfg.width,
            cfg.height,
            SDL_WINDOW_VULKAN | cfg.flags
        );
    }

    ////
    // ~Window()
    //
    // Custom destructor handles the cleanup up the SDL_Window *.
    Window::~Window() {
        SDL_DestroyWindow(window);
        SDL_Vulkan_UnloadLibrary();
        SDL_Quit();
    }

    ////
    // SDL_Window *ref()
    //
    // Returns a reference to the internal SDL window attached to this
    // object.
    SDL_Window *Window::ref() { return window; }
}
