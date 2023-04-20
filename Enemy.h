#pragma once
#include "Common.h"
#include "Entity.h"

class Enemy: public Entity
{
    public:
        Enemy() = default;
        Enemy(unsigned id, float x, float y);
        virtual ~Enemy() = default;
        
        virtual void move(Move direction) override;
        virtual void draw(sf::RenderWindow&) override;
        Move get_travel_direction() const {return travel_direction;}
    private:
        Move travel_direction = Move::Right;
};

Enemy::Enemy(unsigned id, float x, float y)
    : Entity()
{
    set_id(id);
    m_render_view.setPosition(sf::Vector2f(x,y));
}

void Enemy::move(Move direction)
{
    switch(direction)
    {
        case Up:
            m_render_view.setPosition(sf::Vector2f(0.f, -0.5f));
            break;
        case Down:
            m_render_view.setPosition(sf::Vector2f(0.f, 0.5f));
            break;
        case Left:
            m_render_view.setPosition(sf::Vector2f(-0.5f, 0.f));        
            break;
        case Right:
            m_render_view.setPosition(sf::Vector2f(0.5f, 0.f));
            break; 
    }
}

void Enemy::draw(sf::RenderWindow& window)
{
    window.draw(m_render_view);
}