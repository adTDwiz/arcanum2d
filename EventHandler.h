#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <SDL.h>

class EventHandler {
public:
    void handleEvents(bool& isGameRunning);
};

#endif // EVENTHANDLER_H