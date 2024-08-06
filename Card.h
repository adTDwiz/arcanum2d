#ifndef CARD_H
#define CARD_H

#include <string>
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using string = std::string;

class Card {
public:
    string name;
    string arcana;
    int number;
    string period;
    string description;
    int index;
    string meaningsUpright;
    string meaningsReversed;

    // Constructor
    Card(const json& cardData);
};

#endif // CARD_H
