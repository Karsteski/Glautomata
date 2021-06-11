// GLEW loads OpenGL function pointers from the system's graphics drivers.
// glew.h MUST be included before gl.h
// clang-format off
#include "SDL_error.h"
#include "SDL_video.h"
#include <GL/glew.h>
#include <GL/gl.h>

#include <SDL.h>
#include <SDL_opengl.h>
// clang-format on

#include <cstdlib>
#include <iostream>

SDL_Window* window = nullptr;
SDL_GLContext context = nullptr;

constexpr int windowWidth = 1280;
constexpr int windowHeight = 720;

bool Initialize();
int Exit();

int main()
{
    if (!Initialize()) {
        std::exit(EXIT_FAILURE);
    }

    while (true) {

        // Update screen
        SDL_GL_SwapWindow(window);
    }

    Exit();
}

bool Initialize()
{
    bool success = true;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        success = false;
    }
    // Use OpenGL 4.0 Core
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Create window
    window = SDL_CreateWindow("Glautomata - Conway's Game of Life using OpenGL", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cout << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
        success = false;
    }

    // Create context
    context = SDL_GL_CreateContext(window);
    if (context == nullptr)

    {
        std::cout << "OpenGL context could not be created! SDL Error: " << SDL_GetError() << std::endl;
        success = false;
    }

    // Initialize GLEW
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        std::cout << "Error initializing GLEW! Error: " << glewGetErrorString(glewError) << std::endl;
    }

    // Use VSync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        std::cout << "Warning: Unable to set VSync! SDL Error: " << SDL_GetError() << std::endl;
    }

    return success;
}

int Exit()
{
    SDL_DestroyWindow(window);
    window = nullptr;
    
    // Quit SDL subsystems
    SDL_Quit();

    std::exit(EXIT_SUCCESS);
}