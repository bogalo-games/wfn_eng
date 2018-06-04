#define STB_IMAGE_IMPLEMENTATION
#define DEBUG

#include <vulkan/vulkan.h>
#include <SDL.h>
#include <SDL_vulkan.h>
#include <glm/glm.hpp>

#include <functional>
#include <stdexcept>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <cmath>
#include <set>

#include "stb_image.h"

#include "engine.hpp"
#include "sdl.hpp"
#include "vulkan/util.hpp"
#include "vulkan.hpp"

const int WIDTH  = 640;
const int HEIGHT = 480;

using namespace wfn_eng;
using namespace wfn_eng::engine;
using namespace wfn_eng::sdl;
using namespace wfn_eng::vulkan;
using namespace wfn_eng::vulkan::util;

////
// vertPath
// fragPath
//
// Paths to both the vertex and fragment shaders.
const static std::string vertPath = "src/shaders/vert.spv";
const static std::string fragPath = "src/shaders/frag.spv";

class HelloTriangleApplication {
private:
    Window *window;

    PrimitiveRenderer *renderer;

    // Vertex definition
    const std::vector<Vertex> vertices = {
        { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f } },
        { {  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f } },
        { {  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f } },
        { { -0.5f,  0.5f }, { 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f } }
    };

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    void init() {
        WindowConfig cfg {
            .vulkanPath = "vulkan/macOS/lib/libvulkan.1.dylib",
            .windowName = "Testing Vulkan",
            .width = WIDTH,
            .height = HEIGHT,
            .flags = 0
        };

        window = new Window(cfg);
        Core::initialize(*window, true);

        renderer = new PrimitiveRenderer(1, 1);
    }

    ////
    // Destroying everything
    void cleanup() {
        delete renderer;

        Core::destroy();
        delete window;
    }

    ////
    // Game Logic
    void updatePosition(const glm::vec2& pos, uint32_t ticks) {
        float t = ticks / 1000.0f;

        std::array<Vertex, 4> mv_verts;
        for (int i = 0; i < mv_verts.size(); i++) {
            float dx = cos(t * (1 + i)) / 8;
            float dy = cos(t * 2 * (1 + i)) / 8;

            mv_verts[i] = vertices[i];
            mv_verts[i].pos = mv_verts[i].pos + pos + glm::vec2(dx , dy);
        }

        renderer->drawQuad(mv_verts);

        std::array<Vertex, 3> triPoints = {
          Vertex { { -1.0f, -1.0f }, { 1.0f, 0.0f, 0.0f }, { 0.0, 0.0 } },
          Vertex { {  1.0f, -1.0f }, { 0.0f, 1.0f, 0.0f }, { 1.0, 0.0 } },
          Vertex { {  0.0f,  1.0f }, { 0.0f, 0.0f, 1.0f }, { 0.5, 1.0 } }
        };

        renderer->drawTriangle(triPoints);
    }

    void mainLoop() {
        bool quit = false;
        SDL_Event event;

        bool up = false, down = false, left = false, right = false;
        glm::vec2 pos(0, 0);

        uint32_t last = 0;
        while (true) {
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT)
                    quit = true;
                else if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_w) up = true;
                    if (event.key.keysym.sym == SDLK_s) down = true;
                    if (event.key.keysym.sym == SDLK_a) left = true;
                    if (event.key.keysym.sym == SDLK_d) right = true;
                } else if (event.type == SDL_KEYUP) {
                    if (event.key.keysym.sym == SDLK_w) up = false;
                    if (event.key.keysym.sym == SDLK_s) down = false;
                    if (event.key.keysym.sym == SDLK_a) left = false;
                    if (event.key.keysym.sym == SDLK_d) right = false;
                }
            }

            uint32_t curr = SDL_GetTicks();

            int dx = 0;
            if (left)  dx -= 1;
            if (right) dx += 1;

            int dy = 0;
            if (up)   dy -= 1;
            if (down) dy += 1;

            int dt = (curr - last);
            pos += glm::vec2(dx, dy) * (dt / 1000.f);

            if (quit == true)
                break;

            updatePosition(pos, curr);
            renderer->render();
            last = curr;
        }

        vkDeviceWaitIdle(Core::instance().device().logical());
    }

public:
    void run() {
        init();
        mainLoop();
        cleanup();
    }
};

////
// void configEnv()
//
// Configures environment variables for macOS.
void configEnv() {
    // TODO: Make this cross platform
    char cwd[FILENAME_MAX];
    getcwd(cwd, FILENAME_MAX);

    std::stringstream pathBuilder;

    pathBuilder << cwd << "/vulkan/macOS/etc/vulkan/icd.d/MoltenVK_icd.json";
    std::string icd = pathBuilder.str();

    pathBuilder.str(std::string());

    pathBuilder << cwd << "/vulkan/macOS/etc/vulkan/explicit_layer.d";
    std::string lyr = pathBuilder.str();

    setenv("VK_ICD_FILENAMES", icd.c_str(), true);
    setenv("VK_LAYER_PATH",    lyr.c_str(), true);
}

////
// int main()
//
// Entry point for the application.
int main() {
    configEnv();

    HelloTriangleApplication app;
    try {
        app.run();
    } catch (const std::runtime_error& e) {
        std::cerr << "App error: " << e.what() << std::endl;
        return 1;
    } catch (const WfnError& e) {
        std::cout << e.what() << std::endl;
        return 1;
    }

    return 0;
}
