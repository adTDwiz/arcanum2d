#include <iostream>
#include <string>
#include <fstream>
#include <algorithm>
#include <random>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <math.h>
#include <ctime>
#include <sstream>

#include <Windows.h>

#include <nlohmann/json.hpp>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "EventHandler.h"
#include "GUIBox.h"
#include "TransitionTimer.h"


using json = nlohmann::json;
using string = std::string;



enum class GameModeState {
    Start_Menu,
    Main_Menu,
    Play_Mode,
    Study_Mode,
    Setting_Menu,
};

//This will keep track of what game state we are in. It should be global.
GameModeState currentGameState = GameModeState::Start_Menu;

//This updates our game state. Called whenever screens change
void updateGameState(GameModeState newState) {
    currentGameState = newState;
}



json readCardData() {


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
    return retrievedCardData;
}




//This is the data for the game mode
struct GameMode {
    std::string name;
    std::string description;
    int noOfCards;
    bool active; // Are we currently using this gametype for play?

    // Constructor to initialize the struct
    GameMode(const std::string& name, const std::string& description, int noOfCards, bool active)
        : name(name), description(description), noOfCards(noOfCards), active(active) {
    }
};

struct Deck {
    std::string name;
    std::string description;
    std::string era;
    std::string difficulty;
    int decksize;
    bool active; // Are we currently using this deck for play?

    // Constructor to initialize the struct
    Deck(const std::string& name, const std::string& description, const std::string& era, const std::string& difficulty, int decksize, bool active)
        : name(name), description(description), era(era), difficulty(difficulty), decksize(decksize), active(active) {
    }
};

class Card {
public:
    std::string name;
    std::string arcana;
    int number;
    std::string period;
    std::string description;
    int index;
    std::string uprightMeaning;
    std::string reversedMeaning;
    std::string hebrew; // Optional
    std::string symbol; // Optional
    std::string element; // Optional

    // Constructor with various parameters
    Card(const std::string& name, const std::string& arcana, int number, const std::string& period,
        const std::string& description, int index, const std::string& uprightMeaning,
        const std::string& reversedMeaning, const std::string& element = "",
        const std::string& hebrew = "", const std::string& symbol = "")
        : name(name), arcana(arcana), number(number), period(period), description(description),
        index(index), uprightMeaning(uprightMeaning), reversedMeaning(reversedMeaning),
        hebrew(hebrew), symbol(symbol), element(element) {
    }

    // This is a method to display card information including element
    void display() const {
        std::cout << "Name: " << name << "\nArcana: " << arcana << "\nNumber: " << number << "\nPeriod: " << period << "\nDescription: " << description << "\nIndex: " << index << "\nUpright Meaning: " << uprightMeaning << "\nReversed Meaning: " << reversedMeaning;
        if (!element.empty()) {
            std::cout << "\nElement: " << element;
        }
        if (!hebrew.empty()) {
            std::cout << "\nHebrew: " << hebrew;
        }
        if (!symbol.empty()) {
            std::cout << "\nSymbol: " << symbol;
        }
        std::cout << "\n";
    }
};

//These are for the node graph
struct Node {
    Card* card;
    std::vector<std::pair<Node*, double>> edges; // Pair of adjacent node and weight
};

class Graph {
public:
    void addNode(Card* card) {
        nodes.push_back({ card, {} });
    }

    void addEdge(Card* from, Card* to, double weight) {
        Node* fromNode = findNode(from);
        Node* toNode = findNode(to);
        if (fromNode && toNode) {
            fromNode->edges.push_back({ toNode, weight });
        }
    }

    void printGraph() const {
        for (const auto& node : nodes) {
            std::cout << "Card: " << node.card->name << " [" << node.card->index << "]\n";
            for (const auto& edge : node.edges) {
                std::cout << "  -> " << edge.first->card->name << " [" << edge.first->card->index << "] (Weight: " << edge.second << ")\n";
            }
        }
    }

    double getEdgeWeight(Card* from, Card* to) const {
        const Node* fromNode = findNode(from);
        if (fromNode) {
            for (const auto& edge : fromNode->edges) {
                if (edge.first->card == to) {
                    return edge.second;
                }
            }
        }
        return -1; // Or any appropriate default value indicating edge not found
    }

    void updateEdgeWeight(Card* from, Card* to, double newWeight) {
        Node* fromNode = findNode(from);
        if (fromNode) {
            for (auto& edge : fromNode->edges) {
                if (edge.first->card == to) {
                    edge.second = newWeight;
                    return;
                }
            }
        }
        // Handle case where edge is not found (optional)
    }

private:
    std::vector<Node> nodes;

    Node* findNode(Card* card) {
        for (auto& node : nodes) {
            if (node.card == card) {
                return &node;
            }
        }
        return nullptr;
    }

    const Node* findNode(Card* card) const {
        for (const auto& node : nodes) {
            if (node.card == card) {
                return &node;
            }
        }
        return nullptr;
    }
};


// Function to calculate Sun sign and Moon phase based on current day using moonInfo data
std::string getMoonPhaseForDate(const std::string& jsonFileName, int year, int month, int day) {
    // Open JSON file
    std::ifstream ifs(jsonFileName);
    if (!ifs.is_open()) {
        std::cerr << "Failed to open JSON file: " << jsonFileName << std::endl;
        return "";
    }

    // Parse JSON content
    json root;
    try {
        ifs >> root;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
        ifs.close();
        return "";
    }
    ifs.close();

    // Find the specified year in JSON
    auto yearData = root[std::to_string(year)];
    if (yearData.is_null()) {
        std::cerr << "Data for year " << year << " not found in JSON" << std::endl;
        return "";
    }

    // Get events array for the specified year
    auto events = yearData["events"];
    if (events.is_null() || !events.is_array()) {
        std::cerr << "Events data not found or not in correct format for year " << year << std::endl;
        return "";
    }

    // Vector to store filtered events
    std::vector<json> filteredEvents;

    // Iterate through events to filter by month and day
    for (const auto& event : events) {
        std::string eventDate = event["date"];
        std::tm event_tm = {};
        std::stringstream ss(eventDate);
        ss >> std::get_time(&event_tm, "%Y-%m-%d");

        if (ss.fail()) {
            std::cerr << "Failed to parse date string: " << eventDate << std::endl;
            continue;  // Skip to next event if date parsing fails
        }

        // Check if event date matches the specified month and is on or before the specified day
        if (event_tm.tm_mon + 1 == month && event_tm.tm_mday <= day) {
            filteredEvents.push_back(event);
        }
    }

    // If no events match the criteria, return empty string
    if (filteredEvents.empty()) {
        std::cerr << "No matching lunar events found for date: " << year << "-" << month << "-" << day << std::endl;
        return "";
    }

    // Sort filtered events by date (ascending order)
    std::sort(filteredEvents.begin(), filteredEvents.end(), [](const json& a, const json& b) {
        std::tm tm_a = {}, tm_b = {};
        std::stringstream ss_a(a["date"].get<std::string>());
        std::stringstream ss_b(b["date"].get<std::string>());
        ss_a >> std::get_time(&tm_a, "%Y-%m-%d");
        ss_b >> std::get_time(&tm_b, "%Y-%m-%d");
        return std::mktime(&tm_a) < std::mktime(&tm_b);
        });

    // Get the closest event (last in sorted list)
    json closestEvent = filteredEvents.back();
    std::string moonPhase = closestEvent["phase"];

    return moonPhase;
}

//This gets the current solar sign
std::string getCurrentSolarSign() {
    time_t t = time(0);
    struct tm now;
    localtime_s(&now, &t);

    int day = now.tm_mday;
    int month = now.tm_mon + 1;  // tm_mon is 0-11, so add 1 for 1-12

    if ((month == 1 && day >= 20) || (month == 2 && day <= 18)) return "Aquarius";
    if ((month == 2 && day >= 19) || (month == 3 && day <= 20)) return "Pisces";
    if ((month == 3 && day >= 21) || (month == 4 && day <= 19)) return "Aries";
    if ((month == 4 && day >= 20) || (month == 5 && day <= 20)) return "Taurus";
    if ((month == 5 && day >= 21) || (month == 6 && day <= 20)) return "Gemini";
    if ((month == 6 && day >= 21) || (month == 7 && day <= 22)) return "Cancer";
    if ((month == 7 && day >= 23) || (month == 8 && day <= 22)) return "Leo";
    if ((month == 8 && day >= 23) || (month == 9 && day <= 22)) return "Virgo";
    if ((month == 9 && day >= 23) || (month == 10 && day <= 22)) return "Libra";
    if ((month == 10 && day >= 23) || (month == 11 && day <= 21)) return "Scorpio";
    if ((month == 11 && day >= 22) || (month == 12 && day <= 21)) return "Sagittarius";
    if ((month == 12 && day >= 22) || (month == 1 && day <= 19)) return "Capricorn";

    return "Unknown";
}


//this is where game logic will be handled
int playMode(const json& retrievedCardData) {

    std::cout << "Play mode selected.\n";

    // Extract the gameModes array from the JSON data
    std::vector<json> game_modes = retrievedCardData.at("gameModes").get<std::vector<json>>();

    // Create a list of GameMode objects
    std::vector<GameMode> gameModeList;
    for (const auto& game_mode : game_modes) {
        std::string name = game_mode.at("name").get<std::string>();
        std::string description = game_mode.at("description").get<std::string>();
        int noOfCards = std::stoi(game_mode.at("noOfCards").get<std::string>());
        bool active = false;

        // Create a GameMode object and add it to the list
        gameModeList.emplace_back(name, description, noOfCards, active);
    }

    /*
    // For demonstration purposes, print the gameModeList. This will print out every one
    std::cout << "Game Mode List:\n";
    for (const auto& gameMode : gameModeList) {
        std::cout << "Name: " << gameMode.name << ", Description: " << gameMode.description << ", Number of Cards: " << gameMode.noOfCards << "\n";
    }
    */

    //We will set the game type here
    int selectedGameModeIndex = 1;

    // This will create the currentGameMode object based on the selected index
    GameMode currentGameMode(
        gameModeList[selectedGameModeIndex].name,
        gameModeList[selectedGameModeIndex].description,
        gameModeList[selectedGameModeIndex].noOfCards,
        true // We are using this gamemode so we'll mark it active
    );
    std::cout << "Selected Game Mode:\n";
    std::cout << "Name: " << currentGameMode.name << ", Description: " << currentGameMode.description << ", Number of Cards: " << currentGameMode.noOfCards << "\n";
    //Here we check if the created gameMode is active
    if (currentGameMode.active) {
        std::cout << "The current game mode is active and we are ready to play.\n";
    }


    // Extract the decks array from the JSON data
    std::vector<json> decks = retrievedCardData.at("decks").get<std::vector<json>>();

    // Create a list of Deck objects
    std::vector<Deck> deckList;
    for (const auto& deck : decks) {
        std::string name = deck.at("name").get<std::string>();
        std::string description = deck.contains("description") ? deck.at("description").get<std::string>() : "Description not provided";
        std::string era = deck.contains("era") ? deck.at("era").get<std::string>() : "Era not provided";
        std::string difficulty = deck.contains("difficulty") ? deck.at("difficulty").get<std::string>() : "Difficulty not provided";
        int decksize = deck.at("cards").size();
        bool active = false;

        // Create a Deck object and add it to the list
        deckList.emplace_back(name, description, era, difficulty, decksize, active);
    }

    int selectedDeckIndex = 0;

    Deck currentDeck(
        deckList[selectedDeckIndex].name,
        deckList[selectedDeckIndex].description,
        deckList[selectedDeckIndex].era,
        deckList[selectedDeckIndex].difficulty,
        deckList[selectedDeckIndex].decksize,
        true // We are using this deck so we'll mark it active
    );

    std::cout << "Selected Deck:\n";
    std::cout << "Name: " << currentDeck.name << ", Description: " << currentDeck.description << ", Era: " << currentDeck.era << ", Difficulty: " << currentDeck.difficulty << ", Decksize: " << currentDeck.decksize << "\n";

    // Here we check if the created deck is active
    if (currentDeck.active) {
        std::cout << "The current deck is active and we are ready to play.\n";
    }
    
    
    // Create a list of cards from the current deck that matches the length specified by the current game mode's noOfCards
    std::vector<Card> selectedCards;
    if (decks[selectedDeckIndex].contains("cards")) {
        std::vector<json> cards = decks[selectedDeckIndex].at("cards").get<std::vector<json>>();
        for (int i = 0; i < cards.size(); ++i) {
            const auto& card = cards[i];
            std::string cardName = card.at("name").get<std::string>();
            std::string arcana = card.at("arcana").get<std::string>();
            int number = card.at("number").get<int>();
            std::string period = card.at("period").get<std::string>();
            std::string cardDescription = card.contains("description") ? card.at("description").get<std::string>() : "";
            int index = card.at("index").get<int>();

            // Initialize optional parameters (default to empty strings)
            std::string uprightMeaning = "";
            std::string reversedMeaning = "";
            std::string hebrew = "";
            std::string symbol = "";
            std::string element = ""; // Initialize element field

            // Check and retrieve optional fields if they exist in JSON
            if (card.contains("meanings")) {
                const auto& meanings = card["meanings"];
                uprightMeaning = meanings.contains("upright") ? meanings["upright"].get<std::string>() : "";
                reversedMeaning = meanings.contains("reversed") ? meanings["reversed"].get<std::string>() : "";
            }
            if (card.contains("Hebrew")) {
                hebrew = card["Hebrew"].get<std::string>();
            }
            if (card.contains("Symbol")) {
                symbol = card["Symbol"].get<std::string>();
            }
            if (card.contains("element")) { // Check for "element" field
                element = card["element"].get<std::string>();
            }

            // Create Card object and add to selectedCards vector
            selectedCards.emplace_back(cardName, arcana, number, period, cardDescription, index,
                uprightMeaning, reversedMeaning, hebrew, symbol, element);
        }
    }

    // Print out the selected cards
    std::cout << "Selected Cards:\n";
    for (const auto& card : selectedCards) {
        std::cout << "Card Index: " << card.index << ", Card Name: " << card.name << "\n";
    }

    // Shuffle the selectedCards vector using std::shuffle with random seed from std::chrono
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(selectedCards.begin(), selectedCards.end(), std::default_random_engine(seed));

    // Print out the selected cards
    std::cout << "Selected Cards (Shuffled):\n";
    for (const auto& card : selectedCards) {
        std::cout << "Card Index: " << card.index << ", Card Name: " << card.name << "\n";
    }

    // Create the graph and add nodes and edges
    Graph graph;
    for (auto& card : selectedCards) {
        graph.addNode(&card);
    }

    // For demonstration, we'll add edges with random weights
    if (selectedCards.size() > 1) {
        for (size_t i = 0; i < selectedCards.size() - 1; ++i) {
            graph.addEdge(&selectedCards[i], &selectedCards[i + 1], static_cast<double>(i + 1) * 0.5);
        }
    }

    // Print the graph
    std::cout << "Graph:\n";
    graph.printGraph();



    ///////////////////////////////////////////////////////////////////////
    ///It's astrology time! -----------------------------------------------
    ///////////////////////////////////////////////////////////////////////
    
    //This will grab the solar sign
    std::string solarSign = getCurrentSolarSign();
    std::cout << "The current solar sign is: " << solarSign << std::endl;

    //This will grab the lunar phase and current time
    string moonData = "moonInfo.json";
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm now_tm;
    localtime_s(&now_tm, &now_time);
    int year = now_tm.tm_year + 1900;
    int month = now_tm.tm_mon + 1;
    int day = now_tm.tm_mday;

    std::string moonPhase = getMoonPhaseForDate(moonData, year, month, day);
    if (!moonPhase.empty()) {
        std::cout << "Moon phase on " << year << "-" << std::setw(2) << std::setfill('0') << month << "-" << std::setw(2) << std::setfill('0') << day << ": " << moonPhase << std::endl;
    }
    else {
        std::cout << "Moon phase not found for " << year << "-" << std::setw(2) << std::setfill('0') << month << "-" << std::setw(2) << std::setfill('0') << day << std::endl;
    }

    // Element compatibility influences on edge weights
    std::map<std::string, std::vector<std::string>> elementCompatibility = {
        {"Fire", {"Air", "Fire"}},
        {"Water", {"Earth", "Water"}},
        {"Air", {"Fire", "Air"}},
        {"Earth", {"Water", "Earth"}}
    };

    // Apply element compatibility influence
    for (size_t i = 0; i < selectedCards.size() - 1; ++i) {
        std::string element1 = selectedCards[i].element;
        std::string element2 = selectedCards[i + 1].element;
        double weight = graph.getEdgeWeight(&selectedCards[i], &selectedCards[i + 1]);

        if (std::find(elementCompatibility[element1].begin(), elementCompatibility[element1].end(), element2) != elementCompatibility[element1].end()) {
            weight *= 0.9; // Reduces weight for compatible elements
        }
        else {
            weight *= 1.1; // Increases weight for incompatible elements
        }

        graph.updateEdgeWeight(&selectedCards[i], &selectedCards[i + 1], weight);
    }

    // Apply lunar and solar modifiers
    double lunarModifier = (moonPhase == "Full Moon") ? 0.8 : 1.2;
    double solarModifier = (solarSign == "Aries") ? 0.85 : 1.15;

    for (size_t i = 0; i < selectedCards.size() - 1; ++i) {
        double weight = graph.getEdgeWeight(&selectedCards[i], &selectedCards[i + 1]);
        weight *= lunarModifier * solarModifier;
        graph.updateEdgeWeight(&selectedCards[i], &selectedCards[i + 1], weight);
    }

    // Print the modified graph
    std::cout << "Modified Graph with Element and Astrology Influences:\n";
    graph.printGraph();


    return 0;
}

//this is the setup function
bool initSDL(SDL_Window*& window, SDL_Renderer*& renderer, int screenWidth, int screenHeight) {
    // Here we will initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Creating a window
    window = SDL_CreateWindow("Arcana Arcade", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenWidth, screenHeight, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create renderer for window
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }

    // Initialize SDL_ttf
    if (TTF_Init() == -1) {
        std::cerr << "SDL_ttf could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }


    // Set renderer draw color to black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    return true;
}

//Function to load images as a package
bool loadTextures(SDL_Renderer* renderer, std::map<std::string, SDL_Texture*>& textures, const std::map<std::string, std::string>& texturePaths) {
    for (const auto& pair : texturePaths) {
        // Load image at specified path
        SDL_Texture* newTexture = IMG_LoadTexture(renderer, pair.second.c_str());
        if (newTexture == nullptr) {
            std::cerr << "Unable to load texture " << pair.second << "! SDL_image Error: " << IMG_GetError() << std::endl;
            return false; // If any texture fails to load, return false
        }
        textures[pair.first] = newTexture;
    }
    return true;
}

// Function to load fonts as a package
void loadFonts(const std::map<std::string, std::string>& fontPaths, std::map<std::string, TTF_Font*>& loadedFonts) {
    for (const auto& font : fontPaths) {
        TTF_Font* loadedFont = TTF_OpenFont(font.second.c_str(), 24); // Adjust font size as needed
        if (loadedFont != nullptr) {
            loadedFonts[font.first] = loadedFont;
        }
        else {
            std::cerr << "Failed to load font: " << font.first << " from path: " << font.second << std::endl;
        }
    }
}

//Load Text
SDL_Texture* loadTextTexture(const std::string& text, TTF_Font* font, SDL_Color color, SDL_Renderer* renderer) {
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, text.c_str(), color);
    if (textSurface == nullptr) {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture == nullptr) {
        std::cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
    }

    SDL_FreeSurface(textSurface);
    return textTexture;
}

//This is for GUI hoverboxes
void renderHoverBoxWithText(SDL_Renderer* renderer, int mouseX, int mouseY, int boxWidth, int boxHeight, const std::string& text, TTF_Font* font, bool ifGradient, SDL_Color startColour, SDL_Color endColour, SDL_Color textColor) {
    // Calculate dimensions for the text texture
    int textWidth, textHeight;
    TTF_SizeText(font, text.c_str(), &textWidth, &textHeight);

    // Calculate dimensions for the hover box around the text texture
    int hoverBoxWidth = textWidth + 20;  // Example: 20 pixels larger than text width
    int hoverBoxHeight = textHeight + 20; // Example: 20 pixels larger than text height

    // Define the hover box rectangle and adjust its position to lower right of cursor
    SDL_Rect hoverBox = { mouseX, mouseY, hoverBoxWidth, hoverBoxHeight };

    // Render shadow box (black color)
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect shadowBox = { hoverBox.x + 5, hoverBox.y + 5, hoverBox.w, hoverBox.h };
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 128); // Black with alpha transparency
    SDL_RenderFillRect(renderer, &shadowBox);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Render main hover box. This also creates the gradient
    if (ifGradient) {
        for (int i = 0; i < hoverBox.w; ++i) {
            float t = (float)i / (hoverBox.w - 1); // Calculate interpolation factor
            Uint8 r = (1 - t) * startColour.r + t * endColour.r;
            Uint8 g = (1 - t) * startColour.g + t * endColour.g;
            Uint8 b = (1 - t) * startColour.b + t * endColour.b;
            Uint8 a = (1 - t) * startColour.a + t * endColour.a;

            SDL_SetRenderDrawColor(renderer, r, g, b, a);
            SDL_RenderDrawLine(renderer, hoverBox.x + i, hoverBox.y, hoverBox.x + i, hoverBox.y + hoverBox.h - 1);
        }
    }
    else {
        SDL_SetRenderDrawColor(renderer, startColour.r, startColour.g, startColour.b, startColour.a);
        SDL_RenderFillRect(renderer, &hoverBox);
    }

    // Render transparent edges (top and left)
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 124); // Semi-transparent white

    // Adjust bright edge highlights
    SDL_Rect topEdge = { hoverBox.x + 3, hoverBox.y, hoverBox.w - 4, 3 }; // Move over by four pixels and reduce length by six pixels
    SDL_Rect leftEdge = { hoverBox.x, hoverBox.y + 0, 3, hoverBox.h - 1 }; // Move down by three pixels and reduce height by six pixels
    SDL_RenderFillRect(renderer, &topEdge);
    SDL_RenderFillRect(renderer, &leftEdge);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Render dark edges (bottom and right)
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 124); // Semi-transparent black
    SDL_Rect bottomEdge = { hoverBox.x + 0, hoverBox.y + hoverBox.h - 3, hoverBox.w - 0, 3 }; // Move over by three pixels and reduce length by three pixels
    SDL_Rect rightEdge = { hoverBox.x + hoverBox.w - 3, hoverBox.y + 2, 3, hoverBox.h - 5 }; // Move right by three pixels and reduce height by six pixels
    SDL_RenderFillRect(renderer, &bottomEdge);
    SDL_RenderFillRect(renderer, &rightEdge);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);

    // Set color for text
    SDL_SetRenderDrawColor(renderer, textColor.r, textColor.g, textColor.b, textColor.a);

    // Example: Load and render text texture centered within hover box
    SDL_Texture* hoverBoxTextTexture = loadTextTexture(text, font, textColor, renderer);
    SDL_Rect textRect = { hoverBox.x + (hoverBox.w - textWidth) / 2, hoverBox.y + (hoverBox.h - textHeight) / 2, textWidth, textHeight };
    SDL_RenderCopy(renderer, hoverBoxTextTexture, nullptr, &textRect);
    SDL_DestroyTexture(hoverBoxTextTexture);
}



//This is the main loop
void renderLoop(SDL_Window* window, SDL_Renderer* renderer, std::map<std::string, SDL_Texture*>& textures, std::map<std::string, TTF_Font*>& fonts, int screenWidth, int screenHeight) {
    bool quit = false;
    SDL_Event e;

    // Main menu buttons
    SDL_Rect area1 = { screenWidth / 20, screenHeight / 6, screenWidth / 4, screenHeight / 4 }; // play
    SDL_Rect area2 = { screenWidth / 20, screenHeight / 2.5, screenWidth / 4, screenHeight / 4 }; // study
    SDL_Rect area3 = { screenWidth / 20, screenHeight / 1.7, screenWidth / 4, screenHeight / 4 }; //quit
    SDL_Rect area4 = { 3 * screenWidth / 4, 3 * screenHeight / 4, screenWidth / 4, screenHeight / 4 }; //settings



    //This is a pointer to the returned SDF_Texture of our font loading function. It will be universal as the function itself can render any font we want.
    //What this does is create an SDL surface, convert that to a texture, and fetch the texture so we can render it in the window according to the coordinates of the rect
    SDL_Texture* loadTextTexture(const std::string & text, TTF_Font * font, SDL_Color color, SDL_Renderer * renderer);

    //This is for the GUI hoverbox
    int hoverboxTextWidth = screenWidth / 4;
    int hoverboxTextHeight = screenHeight / 4;

    while (!quit) {
        // Handle events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }

            // These are the mouse events in the Main_Menu
            if (currentGameState == GameModeState::Main_Menu) {
                if (e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN) {
                    int mouseX = e.motion.x;
                    int mouseY = e.motion.y;
                    SDL_Point mousePoint = { mouseX, mouseY };

                    if (SDL_PointInRect(&mousePoint, &area1)) {
                        if (e.type == SDL_MOUSEMOTION) {
                            // Mouse is over area1
                            //std::cout << "Mouse over area 1" << std::endl;
                        }
                        if (e.type == SDL_MOUSEBUTTONDOWN) {
                            // Mouse clicked in area1
                            std::cout << "Mouse clicked in area 1" << std::endl;
                            currentGameState = GameModeState::Play_Mode;
                        }
                    }

                    if (SDL_PointInRect(&mousePoint, &area2)) {
                        if (e.type == SDL_MOUSEMOTION) {
                            // Mouse is over area2
                            //std::cout << "Mouse over area 2" << std::endl;
                        }
                        if (e.type == SDL_MOUSEBUTTONDOWN) {
                            // Mouse clicked in area2
                            std::cout << "Mouse clicked in area 2" << std::endl;
                        }
                    }

                    if (SDL_PointInRect(&mousePoint, &area3)) {
                        if (e.type == SDL_MOUSEMOTION) {
                            // Mouse is over area3
                            //std::cout << "Mouse over area 3" << std::endl;
                        }
                        if (e.type == SDL_MOUSEBUTTONDOWN) {
                            // Mouse clicked in area3
                            std::cout << "Mouse clicked in area 3" << std::endl;
                        }
                    }

                    if (SDL_PointInRect(&mousePoint, &area4)) {
                        if (e.type == SDL_MOUSEMOTION) {
                            // Mouse is over area4
                            //std::cout << "Mouse over area 4" << std::endl;
                        }
                        if (e.type == SDL_MOUSEBUTTONDOWN) {
                            // Mouse clicked in area4
                            std::cout << "Mouse clicked in area 4" << std::endl;
                            currentGameState = GameModeState::Setting_Menu;
                        }
                    }
                }
            }

            // These are the mouse events in the Start_Menu
            if (currentGameState == GameModeState::Start_Menu) {
                if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                    // Change game state to Main_Menu when left mouse button is clicked
                    currentGameState = GameModeState::Main_Menu;
                }
            }

            // These are the mouse events in the Settings Menu
            if (currentGameState == GameModeState::Setting_Menu)
            {
                //Mouse Events for settings
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Black
        SDL_RenderClear(renderer);

        switch (currentGameState) {
            case GameModeState::Start_Menu: {
                // Render the background texture
                SDL_RenderCopy(renderer, textures["startMenuBG"], nullptr, nullptr);

                // Render title logo
                SDL_Rect titleLogo = { screenWidth / 4, screenHeight / 8, screenWidth / 2, screenHeight / 1.5846153846154 };
                SDL_RenderCopy(renderer, textures["titleLogo"], nullptr, &titleLogo);

                // Render text
                SDL_Color textColor = { 255, 255, 255 }; // White color
                SDL_Texture* textTexture = loadTextTexture("Click to start!", fonts["alagard"], textColor, renderer);
                int textWidth = screenWidth / 2.1;
                int textHeight = screenHeight / 7.2;
                int textX = (screenWidth - textWidth) / 2;
                int textY = (screenHeight - textHeight) / 1.1;
                SDL_Rect textRect = { textX, textY, textWidth, textHeight };
                SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

                SDL_DestroyTexture(textTexture);

                break;
            }
            case GameModeState::Main_Menu: {
                // Render the background texture
                SDL_RenderCopy(renderer, textures["mainMenuBG"], nullptr, nullptr);

                

                // Render text
                int textWidth = screenWidth / 6.3;
                int textHeight = screenHeight / 21.6;
                int textX = (screenWidth - textWidth) / 2;
                int textY = (screenHeight - textHeight) / 1.1;
                SDL_Rect textRect = { textX, textY, textWidth, textHeight };

                SDL_RenderCopy(renderer, textures["playButton"], nullptr, &area1);
                SDL_RenderCopy(renderer, textures["studyButton"], nullptr, &area2);
                SDL_RenderCopy(renderer, textures["quitButton"], nullptr, &area3);

                SDL_Color textColor = { 255, 255, 255 }; // White color
                SDL_Texture* textTexture = loadTextTexture("Select an option", fonts["kappa20"], textColor, renderer);
                SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);


                // This is for testing if the mouse is hovering over specific buttons in the Main_Menu
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                SDL_Point mousePoint = { mouseX, mouseY };

                if (SDL_PointInRect(&mousePoint, &area1)) {
                    // Mouse is over area1
                    SDL_Rect playIcon = { screenWidth / 2.2, screenHeight / 6, screenWidth / 2.2, screenHeight / 1.5 };
                    SDL_RenderCopy(renderer, textures["playModeIcon"], nullptr, &playIcon);

                    //This is for the GUI Hoverbox
                    renderHoverBoxWithText(renderer, mouseX + (screenWidth/70), mouseY + (screenHeight/80), screenWidth / 3, screenHeight / 14, "Play a game of Tarot", fonts["kappa20"], 1, { 210, 32, 250, 255 }, { 32, 210, 250, 255 },  textColor);
                }

                if (SDL_PointInRect(&mousePoint, &area2)) {
                    // Mouse is over area2
                    SDL_Rect studyIcon = { screenWidth / 2.2, screenHeight / 6, screenWidth / 2.4, screenHeight / 1.5 };
                    SDL_RenderCopy(renderer, textures["studyModeIcon"], nullptr, &studyIcon);
                    renderHoverBoxWithText(renderer, mouseX + (screenWidth / 70), mouseY + (screenHeight / 40), screenWidth / 3, screenHeight / 14, "Study a card or deck", fonts["kappa20"], 1, { 210, 32, 250, 255 }, { 32, 210, 250, 255 }, textColor);
                    //std::cout << "Mouse over area 2" << std::endl;
                }

                if (SDL_PointInRect(&mousePoint, &area3)) {
                    // Mouse is over area3
                    //std::cout << "Mouse over area 3" << std::endl;
                    renderHoverBoxWithText(renderer, mouseX + (screenWidth / 70), mouseY + (screenHeight / 40), screenWidth / 11, screenHeight / 14, "Exit?", fonts["kappa20"], 1, { 210, 32, 250, 255 }, { 32, 210, 250, 255 }, textColor);

                }

                if (SDL_PointInRect(&mousePoint, &area4)) {
                    // Mouse is over area4
                    //std::cout << "Mouse over area 4" << std::endl;
                    SDL_Rect studyIcon = { screenWidth / 2.2, screenHeight / 6, screenWidth / 2.4, screenHeight / 1.5 };
                    SDL_RenderCopy(renderer, textures["settingModeIcon"], nullptr, &studyIcon);
                    renderHoverBoxWithText(renderer, mouseX - (screenWidth / 5) - (screenWidth / 70), mouseY - (screenHeight / 14) - (screenHeight / 40), screenWidth / 11, screenHeight / 14, "Settings and records", fonts["kappa20"], 1, { 210, 32, 250, 255 }, { 32, 210, 250, 255 }, textColor);
                }

                SDL_DestroyTexture(textTexture);

                break;
            }
            case GameModeState::Play_Mode: {
                
                // Grabbing our card info
                json retrievedCardData = readCardData();
                playMode(retrievedCardData);

                currentGameState = GameModeState::Start_Menu;

                break;
            }
            case GameModeState::Study_Mode: {
                // Render the study mode
                // Add your rendering code for the study mode here
                // Example: SDL_RenderCopy(renderer, textures["studymodeBG"], nullptr, nullptr);
                break;
            }
            case GameModeState::Setting_Menu: {
                static TransitionTimer fadeTimer;
                static bool fadeStarted = false;
                static bool fadeComplete = false; // Track if fade is complete

                // Initialize the fade-in effect
                if (!fadeStarted && !fadeComplete) {
                    fadeTimer.start(2.0f); // 2 seconds for the fade-in effect
                    fadeStarted = true;
                }

                // Render the background texture
                SDL_RenderCopy(renderer, textures["optionMenuBG"], nullptr, nullptr);

                // This is the transparent backing
                GUIBox box(renderer, 100, 100, 200, 150, { 255, 0, 0, 0 }, 1.0f, ElementType::SOLID_SHAPE);
                box.render();

                // Frame for stats
                SDL_Rect frameUI = { (screenWidth - (int)(screenWidth * 0.8f)) / 2, (screenHeight - (int)(screenHeight * 0.8f)) / 2, (int)(screenWidth * 0.8f), (int)(screenHeight * 0.8f) };
                SDL_RenderCopy(renderer, textures["settingsFrameUI"], nullptr, &frameUI);

                // Calculate fade value and render black square
                float fadeValue = fadeTimer.getValue();

                SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND); // Ensure blending is enabled
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, static_cast<Uint8>(255 * (1.0f - fadeValue)));
                SDL_Rect fadeRect = { 0, 0, screenWidth, screenHeight };
                SDL_RenderFillRect(renderer, &fadeRect);

                // Update the fade timer
                fadeTimer.update();

                // Check if fade is complete and handle state
                if (fadeValue >= 1.0f) {
                    fadeStarted = false;
                    fadeComplete = true; // Mark fade as complete

                    // Optionally reset the timer (if it has a reset() method)
                    fadeTimer.reset();
                }
            }
        }

        // Update screen
        SDL_RenderPresent(renderer);
    }
}


//This is the data for the Ollama Listener
struct OllamaData {
    std::mutex mutex;
    std::condition_variable cv;
    bool ready = false;
    std::string listenerCommand;
};

//This waits for commands to be sent to Ollama. It's run on a separate thread.
void OllamaListener(OllamaData& ollamaData) {
    while (true) {
        // Wait for a prompt to be ready
        {
            std::unique_lock<std::mutex> lock(ollamaData.mutex);
            ollamaData.cv.wait(lock, [&] { return ollamaData.ready; });
        }

        // Execute the prompt command
        int result = system(ollamaData.listenerCommand.c_str());
        std::cout << "Command executed with result: " << result << std::endl;

        // Reset ready flag
        {
            std::unique_lock<std::mutex> lock(ollamaData.mutex);
            ollamaData.ready = false;
        }
    }
}

void OllamaPrompt(OllamaData& ollamaData, const std::string& command) {
    {
        std::lock_guard<std::mutex> lock(ollamaData.mutex);
        ollamaData.listenerCommand = command;
        ollamaData.ready = true;
    }
    ollamaData.cv.notify_one();
}

//This is to close the program
void shutdown(SDL_Window* window, SDL_Renderer* renderer, std::map<std::string, TTF_Font*>& fonts, std::thread& ollamaThread, OllamaData& ollamaData) {
    // Close all loaded fonts
    for (auto& font : fonts) {
        TTF_CloseFont(font.second);
    }

    // Send command to stop Ollama
    OllamaPrompt(ollamaData, "ollama stop mistral");

    // Quit SDL_ttf
    TTF_Quit();

    // Quit SDL_image
    IMG_Quit();

    // Destroy renderer and window
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    // Quit SDL
    SDL_Quit();

    // Join Ollama thread
    ollamaThread.join();
}

int main(int argc, char* argv[]) {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    const int screenWidth = 1280;
    const int screenHeight = 720;

    
    //We are running ollama on a separate thread
    OllamaData ollamaData;
    std::thread ollamaThread(OllamaListener, std::ref(ollamaData));
    //This will get ollama running
    OllamaPrompt(ollamaData, "ollama run mistral");
    
    

    // If we can't run SDL, throw error
    if (!initSDL(window, renderer, screenWidth, screenHeight)) {
        std::cerr << "Failed to initialize SDL." << std::endl;
        return -1;
    }

    // Creating event handler, and turning the main loop on
    EventHandler eventHandler;

    

    std::map<std::string, SDL_Texture*> textures;
    std::map<std::string, std::string> texturePaths = {
        {"startMenuBG", "assets/startMenu.png"},
        {"titleLogo", "assets/titleLogo.png"},
        {"mainMenuBG", "assets/mainMenu.png"},
        {"optionMenuBG", "assets/optionMenu.png"},
        {"playButton", "assets/playButton.png"},
        {"studyButton", "assets/studyButton.png"},
        {"quitButton", "assets/quitButton.png"},
        {"playModeIcon", "assets/playModeIcon.png"},
        {"studyModeIcon", "assets/studyModeIcon.png"},
        {"settingModeIcon", "assets/settingModeIcon.png"},
        {"settingsFrameUI", "assets/settingsFrame.png"}
    };

    //Load font links
    std::map<std::string, TTF_Font*> fonts;
    std::map<std::string, std::string> fontPaths = {
        {"alagard", "assets/alagard.ttf"},
        {"kappa20", "assets/kappa20.ttf"}
    };

    // Error check for images
    if (!loadTextures(renderer, textures, texturePaths)) {
        std::cerr << "Failed to load textures." << std::endl;
        return -1;
    }

    loadFonts(fontPaths, fonts);

    renderLoop(window, renderer, textures, fonts, screenWidth, screenHeight);

    shutdown(window, renderer, fonts, ollamaThread, ollamaData);
    return 0;
}

