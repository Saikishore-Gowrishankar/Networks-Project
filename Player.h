#pragma once
#include <SFML/Graphics.hpp>

#include "Common.h"
#include "Entity.h"

#include "Candle/RadialLight.hpp"
#include "Candle/LightingArea.hpp"

class Player final : public Entity
{
    public:
        Player(bool other = false);
        virtual ~Player() = default;
        
        sf::View& get_view() { return m_view; }
        candle::RadialLight& get_flashlight() { return m_flashlight; }

        virtual void move(Move direction) override;
        virtual void draw(sf::RenderWindow&) override;
    private:
        bool m_other; //Set for the local view of other connected players
        sf::View m_view;
        candle::RadialLight m_flashlight;
        float player_velocity = 2.0f;
};
