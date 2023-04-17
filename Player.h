#pragma once
#include <SFML/Graphics.hpp>

#include "Common.h"

class Player
{
    public:
        Player(bool other = false);
        void move(Move direction);
        void set_id(unsigned id){m_id = id;}
        unsigned get_id() const {return m_id;}
        sf::CircleShape& get_player() {return m_player_view;}
        void draw(sf::RenderWindow&);
    private:
        unsigned m_id;
        bool m_other; //Set for the local view of other connected players
        sf::View m_view;
        sf::CircleShape m_player_view;
        float player_velocity = 2.0f;
};
