// GUIBox.cpp
#include "GUIBox.h"

// Constructor to initialize the GUIBox with position, dimensions, color, and opacity
GUIBox::GUIBox(int x, int y, int width, int height, SDL_Color color, float elementOpacity)
    : x_(x), y_(y), width_(width), height_(height), color_(color), opacity_(elementOpacity) {
    // Ensure opacity is between 0.0 and 1.0
    if (opacity_ < 0.0f) opacity_ = 0.0f;
    if (opacity_ > 1.0f) opacity_ = 1.0f;
}

// Set the position of the GUIBox
void GUIBox::setPosition(int x, int y) {
    x_ = x;
    y_ = y;
}

// Set the dimensions of the GUIBox
void GUIBox::setDimensions(int width, int height) {
    width_ = width;
    height_ = height;
}

// Set the color of the GUIBox
void GUIBox::setColor(SDL_Color color) {
    color_ = color;
}

// Set the opacity of the GUIBox
void GUIBox::setOpacity(float opacity) {
    opacity_ = opacity;
    // Ensure opacity is between 0.0 and 1.0
    if (opacity_ < 0.0f) opacity_ = 0.0f;
    if (opacity_ > 1.0f) opacity_ = 1.0f;
}

// Render the GUIBox on the screen
void GUIBox::render(SDL_Renderer* renderer) {
    // Set the drawing color, including the opacity (alpha channel)
    SDL_SetRenderDrawColor(renderer, color_.r, color_.g, color_.b, static_cast<Uint8>(255 * opacity_));

    // Define the rectangle
    SDL_Rect rect = { x_, y_, width_, height_ };

    // Render the rectangle
    SDL_RenderFillRect(renderer, &rect);
}