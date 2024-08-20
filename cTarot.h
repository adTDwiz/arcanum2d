#ifndef CTAROT_H
#define CTAROT_H

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>

// Global variables for SDL_Window and SDL_Renderer
extern SDL_Window* Window;
extern SDL_Renderer* Renderer;

// Function declarations
bool initSDL(SDL_Window*& window, SDL_Renderer*& renderer, int screenWidth, int screenHeight);


#endif // CTAROT_H