/*
*	Networks Project
*
*	Copyright 2023 (c) Saikishore Gowrishankar, Dalon Vura, Bryce Haldeman
*	
*       All owned trademarks belong to their respective owners
*	Lawyers love tautologies :)	
*/

//SFML Includes
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>

//Standard Library Includes
#include <iostream>
#include <string>

//Local Dependencies
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
