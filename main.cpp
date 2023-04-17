#include "Game.h"
#include "Client.h"
#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <thread>
#include <iostream>
#include <mutex>
#include <vector>

int main()
{
    Game game("Bozo Project");
    game.run();
    return 0;
}
