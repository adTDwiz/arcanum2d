// GUIBox.h

#ifndef GUIBOX_H
#define GUIBOX_H

#include <SDL.h>
#include <string>

// Enum to define the type of content the GUIBox holds
enum class ElementType {
    SOLID_SHAPE,
    GRADIENT_SHAPE,
    TEXT,
    IMAGE,
    BUTTON
};

class GUIBox {
public:
    GUIBox(SDL_Renderer* renderer, int x, int y, int width, int height, SDL_Color color, float elementOpacity, ElementType type);

    void setPosition(int x, int y);
    void setDimensions(int width, int height);
    void setColor(SDL_Color color);
    void setOpacity(float opacity);
    void setText(const std::string& text);
    void setImage(const std::string& imagePath);
    void render();

private:
    SDL_Renderer* renderer_;  // Store renderer
    int x_, y_, width_, height_;
    SDL_Color color_;
    float opacity_;
    ElementType type_;

    // Additional properties for text and image
    std::string text_;
    SDL_Texture* imageTexture_;

    // Helper methods for rendering different types
    void renderSolidShape();
    void renderGradientShape();
    void renderText();
    void renderImage();
    void renderButton();
};

#endif // GUIBOX_H
