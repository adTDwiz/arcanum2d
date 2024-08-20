#include "GameObjects.h"

// Define the constructor
GameObjectsStruct::GameObjectsStruct(SDL_Renderer* renderer)
    : settingsBackgroundTransparentBox(renderer, 230, 150, 800, 450, { 0, 0, 0, 0 }, 0.3f, ElementType::SOLID_SHAPE),
    settingsBox(renderer, 230, 150, 800, 450, { 0, 0, 0, 0 }, 0.3f, ElementType::SOLID_SHAPE),
    audioSettingsBox(renderer, 230, 150, 800, 450, { 0, 0, 0, 0 }, 0.3f, ElementType::SOLID_SHAPE),
    videoSettingsBox(renderer, 230, 150, 800, 450, { 0, 0, 0, 0 }, 0.3f, ElementType::SOLID_SHAPE),
    creditsBox(renderer, 230, 150, 800, 450, { 0, 0, 0, 0 }, 0.3f, ElementType::SOLID_SHAPE)
{}