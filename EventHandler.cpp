#include "EventHandler.h"
#include <iostream>

void EventHandler::handleEvents(bool& isGameRunning) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Handle quit event
        if (event.type == SDL_QUIT) {
            isGameRunning = false;
        }
        // Handle key press event
        else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                isGameRunning = false;
            }
        }
        // Handle other events here
    }
}
