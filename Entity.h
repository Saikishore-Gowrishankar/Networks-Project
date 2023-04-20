#pragma once
#include <SFML/Graphics.hpp>
#include <memory>

#include "Common.h"

using Object = sf::CircleShape;

class Entity
{
    public:
        Entity() = default;
        virtual ~Entity() = default;
        virtual void move(Move direction) = 0;
        virtual void set_id(unsigned id) {m_id = id;}
        virtual unsigned get_id() const  {return m_id;} 
        virtual void draw(sf::RenderWindow&) = 0;
        virtual Object& get_entity() {return m_render_view;}
    protected:
        unsigned m_id;
        Object m_render_view;
};