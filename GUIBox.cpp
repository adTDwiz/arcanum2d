// GUIBox.cpp

#include "GUIBox.h"
#include <SDL_ttf.h>
#include <SDL_image.h>

// Constructor
GUIBox::GUIBox(SDL_Renderer* renderer, int x, int y, int width, int height, SDL_Color color, float elementOpacity, ElementType type)
    : renderer_(renderer), x_(x), y_(y), width_(width), height_(height), color_(color), opacity_(elementOpacity), type_(type), imageTexture_(nullptr) {
    if (opacity_ < 0.0f) opacity_ = 0.0f;
    if (opacity_ > 1.0f) opacity_ = 1.0f;
}

// Setters
void GUIBox::setPosition(int x, int y) {
    x_ = x;
    y_ = y;
}

void GUIBox::setDimensions(int width, int height) {
    width_ = width;
    height_ = height;
}

void GUIBox::setColor(SDL_Color color) {
    color_ = color;
}

void GUIBox::setOpacity(float opacity) {
    opacity_ = opacity;
    if (opacity_ < 0.0f) opacity_ = 0.0f;
    if (opacity_ > 1.0f) opacity_ = 1.0f;
}

void GUIBox::setText(const std::string& text) {
    text_ = text;
}

void GUIBox::setImage(const std::string& imagePath) {
    if (imageTexture_) {
        SDL_DestroyTexture(imageTexture_);
    }

    SDL_Surface* surface = IMG_Load(imagePath.c_str());
    if (surface) {
        imageTexture_ = SDL_CreateTextureFromSurface(renderer_, surface);
        SDL_FreeSurface(surface);
    }
    else {
        // Handle error
    }
}

// Rendering methods
void GUIBox::render() {
    switch (type_) {
    case ElementType::SOLID_SHAPE:
        renderSolidShape();
        break;
    case ElementType::GRADIENT_SHAPE:
        renderGradientShape();
        break;
    case ElementType::TEXT:
        renderText();
        break;
    case ElementType::IMAGE:
        renderImage();
        break;
    case ElementType::BUTTON:
        renderButton();
        break;
    }
}

void GUIBox::renderSolidShape() {
    SDL_SetRenderDrawColor(renderer_, color_.r, color_.g, color_.b, static_cast<Uint8>(255 * opacity_));
    SDL_Rect rect = { x_, y_, width_, height_ };
    SDL_RenderFillRect(renderer_, &rect);
}

void GUIBox::renderGradientShape() {
    // Implement gradient rendering logic
}

void GUIBox::renderText() {
    if (TTF_Init() == -1) {
        // Handle error
        return;
    }

    TTF_Font* font = TTF_OpenFont("font.ttf", 24); // Use your font path
    if (!font) {
        // Handle error
        return;
    }

    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text_.c_str(), color_);
    if (textSurface) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer_, textSurface);
        SDL_FreeSurface(textSurface);

        int textWidth = textSurface->w;
        int textHeight = textSurface->h;
        SDL_Rect renderQuad = { x_, y_, textWidth, textHeight };
        SDL_RenderCopy(renderer_, textTexture, nullptr, &renderQuad);

        SDL_DestroyTexture(textTexture);
    }

    TTF_CloseFont(font);
    TTF_Quit();
}

void GUIBox::renderImage() {
    if (imageTexture_) {
        SDL_Rect renderQuad = { x_, y_, width_, height_ };
        SDL_RenderCopy(renderer_, imageTexture_, nullptr, &renderQuad);
    }
}

void GUIBox::renderButton() {
    renderSolidShape();
    renderText();
}