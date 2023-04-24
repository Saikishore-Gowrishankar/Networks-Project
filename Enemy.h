#pragma once
#include <sstream>
#include <iostream>
#include "Common.h"
#include "Entity.h"

class Enemy: public Entity
{
    public:
        unsigned m_health = 10;
        sf::Sprite m_monster;
        unsigned m_frame_num = 0;
        float posx, posy;
        Enemy()
        {
        }
        Enemy(unsigned id, float x, float y)
            : Entity(), posx(y), posy(y)
        {
            set_id(id);
            m_render_view.setRadius(11);
            m_render_view.setFillColor(sf::Color::Blue);
            m_render_view.setPosition(sf::Vector2f(x,y));
        }

        virtual void move(Move direction) override
        {
            switch(direction)
            {
                case Up:
                    m_render_view.move(sf::Vector2f(0.f, -0.1f));
                    break;
                case Down:
                    m_render_view.move(sf::Vector2f(0.f, 0.1f));
                    break;
                case Left:
                    m_render_view.move(sf::Vector2f(-0.1f, 0.f));
                    break;
                case Right:
                    m_render_view.move(sf::Vector2f(0.1f, 0.f));
                    break;
            }
        }
        virtual void draw(sf::RenderWindow& window) override
        {
            m_render_view.setRadius(11);
            m_render_view.setFillColor(sf::Color::Blue);
            //window.draw(m_render_view);
            sf::Texture texture;
            std::ostringstream ss;
            ss << m_frame_num++ % 7 << ".png";

            if(!texture.loadFromFile(std::string("Resources/Sprites/dungeon/frames/frame" + ss.str()))) std::cout << "ERROR!\n";
            m_monster.setTexture(texture);
            m_monster.setOrigin(m_monster.getGlobalBounds().width/2.f, m_monster.getGlobalBounds().height/2.f);
            m_monster.setPosition(sf::Vector2f(posx, posy));
            window.draw(m_monster);
        }
        Move get_travel_direction() const {return travel_direction;}
    private:
        Move travel_direction = Move::Right;
};

