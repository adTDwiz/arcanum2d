#include "TransitionTimer.h"

// Constructor initializes the timer to a default state
TransitionTimer::TransitionTimer() : duration(0.0f), startTime(0), running(false) {}

// Starts the timer with the specified duration
void TransitionTimer::start(float duration) {
    this->duration = duration;
    startTime = SDL_GetTicks();
    running = true;
}

// Updates the timer; called once per frame
void TransitionTimer::update() {
    if (running) {
        // Calculate the elapsed time
        Uint32 currentTime = SDL_GetTicks();
        float elapsedTime = (currentTime - startTime) / 1000.0f;

        // Stop running if duration is exceeded
        if (elapsedTime >= duration) {
            running = false;
        }
    }
}

// Returns the current progress as a value between 0.0 and 1.0
float TransitionTimer::getValue() const {
    if (!running) return 1.0f; // If not running, transition is complete

    // Calculate the elapsed time
    Uint32 currentTime = SDL_GetTicks();
    float elapsedTime = (currentTime - startTime) / 1000.0f;

    // Calculate and return the progress as a fraction of duration
    return (elapsedTime / duration > 1.0f) ? 1.0f : elapsedTime / duration;
}

// Resets the timer to its initial state
void TransitionTimer::reset() {
    running = false;
    startTime = 0;
}
