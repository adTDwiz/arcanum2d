#include <iostream>
#include <string>
#include <fstream>

#include <nlohmann/json.hpp>

#include <SDL.h>
#include <SDL_image.h>

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>

using json = nlohmann::json;
using string = std::string;


class Gametype {
public:

    //we are allocating space for a string
    string name;

    //We are creating a gametype class and placing the "name" string variable inside of it, and setting that to whatever modeName is pointing to
    Gametype(const string& modeName) : name(modeName) {
        std::cout << "Gametype constructor called with modeName: " << modeName << std::endl;
    }

};

class Decktype {
public:

    string name;

    Decktype(const string& deckName) : name(deckName) {
        std::cout << "You are playing with a " << deckName << " deck." << std::endl;
    }
};

class Card {
public:
    int indexNo;

    Card(const int cardind) : indexNo(cardind) {
        std::cout << "Your card index is: " << cardind << std::endl;
    }
};



int readCardData() {


    //This is our card data
    string cardDataPath = "cardInfo.json";
    // We are creating an ifstream object named "cardInfo"
    std::ifstream cardInfo(cardDataPath);

    //check if the path opened correctly
    if (!cardInfo.is_open()) {
        std::cerr << "Failed to open card data file: " << cardDataPath << std::endl;
        //typically 1 is returned on errors
        return 1;
    }

    // this is where the ingested card info will be stored, in a json object
    json retrievedCardData;

    //we are testing here if our JSON grab above worked
    try {
        //we are injecting the JSON into our local JSON object
        cardInfo >> retrievedCardData;
    }
    //we are catching any errors and assigning "carderror" to be a reference to the error itself
    catch (const std::exception& carderror) {
        //we are printing the error
        std::cerr << "Error parsing JSON file: " << carderror.what() << std::endl;
        return 1;
    }

    // We are closing the file here because we only need a single grab
    cardInfo.close();

    //////////////////////////////////////////////////
    //Now the data is stored in "retrievedCardData"//
    /////////////////////////////////////////////////
}



int studyMode() {
    return 0;
}

int playMode() {

    std::cout << "Please select a game to play.\n";

    string gameChoice;

    while (true) {
        std::cin >> gameChoice;

        //we are converting every character in here to lowercase to avoid errors
        for (char& c : gameChoice) {
            c = tolower(c);
        }

        //we are seeing what the first character of the input was to determine the result and will loop the question if we do not get an answer
        switch (gameChoice[0]) {
        case '1':
            std::cout << "Celtic Cross selected\n";

            break;
        case '2':
            std::cout << "Tree of Life selected\n";
            break;
        default:
            std::cout << "Sorry, I didn't understand that.\n";
            continue;
        }
        break;
    }


    return 0;
}

int setGameMode() {

    //input the game mode
    std::cout << "Greetings! Would you like to play or study?\n\n";
    string mainMenuChoice;

    while (true) {
        std::cin >> mainMenuChoice;

        //we are converting every character in here to lowercase to avoid errors
        for (char& c : mainMenuChoice) {
            c = tolower(c);
        }

        //we are seeing what the first character of the input was to determine the result and will loop the question if we do not get an answer
        switch (mainMenuChoice[0]) {
        case 'p':
            playMode();
            break;
        case 's':
            std::cout << "Which deck would you like to examine?\n";
            break;
        default:
            std::cout << "Sorry, I didn't understand that.\n";
            continue;
        }
        break;
    }
    return 0;
}

//this is the struct where the SDL instance is setup
struct SDLContext {
    SDL_Window* window;
    SDL_Renderer* renderer;

    SDLContext(SDL_Window* win, SDL_Renderer* rend) : window(win), renderer(rend) {}
};

//This will handle the functions of the SDL framework 
SDLContext SDL_Setup() {

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Hello world", "success", NULL);

    
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    //starting up SDL
    SDL_Init(SDL_INIT_VIDEO);

    //we are creating a window and renderer in the locations of the initial variables
    SDL_CreateWindowAndRenderer(640, 480, 0, &window,&renderer);
    SDL_RenderSetScale(renderer, 4, 4);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_RenderPresent(renderer);

    //SDL_Delay(10000);

    //we are creating a struct instance here and loading the pointers to the window and renderer in it
    SDLContext context(window, renderer);

    //we are passing our newly created struct out
    return context;
}

//here we will load the main menu on startup
int mainMenuLoad(SDLContext& context) {

    SDL_Surface* imageSurface = SDL_LoadBMP("testImg.bmp");
    if (imageSurface == nullptr) {
        std::cout << "Can't load image. Error: " << SDL_GetError() << std::endl;
        return -1; // Return error code
    }

    SDL_Renderer* renderer = context.renderer;
    if (renderer == nullptr) {
        std::cout << "Failed to create renderer. Error: " << SDL_GetError() << std::endl;
        SDL_FreeSurface(imageSurface);
        return -1; // Return error code
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(context.renderer, imageSurface);
    SDL_FreeSurface(imageSurface);
    if (texture == nullptr) {
        std::cout << "Failed to create texture. Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(context.renderer);
        return -1; // Return error code
    }

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);

    return 0; // Return success
}

//UI Setup
int imguiSetup(SDLContext& context) {

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui_ImplSDL2_InitForSDLRenderer(context.window, context.renderer);
    ImGui_ImplSDLRenderer2_Init(context.renderer);


    //create frame
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    ImGui::Begin("test");
    ImGui::Text("Hello world");
    ImGui::End();
    ImGui::Render();


    //Close Renderer
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    


    return 0;
}

int main(int argc, char* argv[]) {

    readCardData();

    //This will be our renderer/window context
    SDLContext g_SDLContext(nullptr, nullptr);

    g_SDLContext = SDL_Setup();

    mainMenuLoad(g_SDLContext);
    imguiSetup(g_SDLContext);

    setGameMode();

    // Create game objects
    Decktype dt("Rider-Waite");
    Card ct(101);
    Gametype gt("CelticCross");

    // Access and print the name member variable
    std::cout << "Gametype name: " << gt.name << std::endl;

    return 0;

}