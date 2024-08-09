// GUIBox.h
#ifndef GUI_BOX_H
#define GUI_BOX_H

#include <SDL.h>

class GUIBox {
public:
    // Constructor to initialize the GUIBox with position, dimensions, and color
    GUIBox(int x, int y, int width, int height, SDL_Color color, float elementOpacity);

    // Set the position of the GUIBox
    void setPosition(int x, int y);

    // Set the dimensions of the GUIBox
    void setDimensions(int width, int height);

    // Set the color of the GUIBox
    void setColor(SDL_Color color);

    // Set the opacity of the GUIBox
    void setOpacity(float opacity);

    // Render the GUIBox on the screen
    void render(SDL_Renderer* renderer);

private:
    int x_, y_;
    int width_, height_;
    SDL_Color color_;
    float opacity_;
};

#endif // GUI_BOX_H