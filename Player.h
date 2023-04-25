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
        bool flashlight_on() const { return m_flashlight_on; }
        void toggle_flashlight() { m_flashlight_on = !m_flashlight_on; }

        virtual void move(Move direction) override;
        virtual void draw(sf::RenderWindow&) override;

        void take_damage()
        {
            if(m_clock.getElapsedTime().asSeconds() >= 1.f)
            {
                m_clock.restart();
                if(--m_health == 0) m_dead = true;
            }
        }
        int get_health() { return m_health; }
        bool is_dead() { return m_dead; }

        bool intersecting = false;
    private:
        bool m_other; //Set for the local view of other connected players
        sf::View m_view;
        candle::RadialLight m_flashlight;
        bool m_flashlight_on = true;
        sf::Text m_name;
        sf::Font m_font;
        float player_velocity = 2.0f;

        sf::Clock m_clock;
        int m_health = 3;
        bool m_dead = false;
};
