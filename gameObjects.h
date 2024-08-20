// GameObjects.h

#ifndef GAMEOBJECTS_H
#define GAMEOBJECTS_H

#include "GUIBox.h"

struct GameObjectsStruct {
    // GUIBox objects
    GUIBox settingsBackgroundTransparentBox;
    GUIBox settingsBox;
    GUIBox audioSettingsBox;
    GUIBox videoSettingsBox;
    GUIBox creditsBox;

    // Constructor to initialize all objects
    GameObjectsStruct(SDL_Renderer* renderer);

    //Failsafe
    GameObjectsStruct() = delete;
};

#endif // GAMEOBJECTS_H