#ifndef TRANSITIONTIMER_H
#define TRANSITIONTIMER_H

#include <SDL.h>

class TransitionTimer {
public:
    TransitionTimer();

    // Starts the timer with the given duration in seconds
    void start(float duration);

    // Updates the timer; should be called each frame
    void update();

    // Returns the current progress as a value between 0.0 and 1.0
    float getValue() const;

    // Resets the timer to its initial state
    void reset();

private:
    float duration;   // Duration of the transition in seconds
    Uint32 startTime; // Start time of the transition
    bool running;     // Whether the timer is currently running
};

#endif // TRANSITIONTIMER_H