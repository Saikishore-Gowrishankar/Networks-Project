#pragma once
#include <SFML/Graphics.hpp>
#include "Entity.h"

class Projectile : public Entity
{
public:
    sf::CircleShape m_bullet;
    sf::Vector2f m_old_pos;
    sf::Vector2f m_current_velocity;
    Projectile()
    {
        m_bullet.setRadius(1.f);
        m_bullet.setFillColor(sf::Color::White);
    }

    virtual void move(Move direction) override{}
    virtual void move(sf::Vector2f pos) { m_bullet.move(pos); }
    virtual void draw(sf::RenderWindow& window) override { window.draw(m_bullet); }

};
