#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include "Game.h"

int main()
{
    std::string ip;
    std::cout << "Enter server IP: ";
    std::cin >> ip;

    Game game("Bozo Project", ip);
    game.run();
    return 0;
}
