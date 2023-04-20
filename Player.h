#pragma once
#include <SFML/Graphics.hpp>

#include "Common.h"
#include "Entity.h"

class Player final : public Entity
{
    public:
        Player(bool other = false);
        virtual ~Player() = default;
        
        sf::View& get_view() { return m_view; }

        virtual void move(Move direction) override;
        virtual void draw(sf::RenderWindow&) override;
    private:
        bool m_other; //Set for the local view of other connected players
        sf::View m_view;
        float player_velocity = 2.0f;
};
