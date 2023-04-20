#include <algorithm>
#include <string>
#include <memory>

#include "Player.h"
#include "Common.h"
#include "Enemy.h"

Player::Player(bool other)
    : m_view(sf::FloatRect(0,0,200,150)), m_other(other)
{
    m_font.loadFromFile("Resources/Fonts/arial.ttf");
    m_name.setFont(m_font);
    m_name.setCharacterSize(20);
    m_render_view.setRadius(5);

    m_flashlight.setRange(50);
    m_flashlight.setFade(false);

    if(other)
        m_render_view.setFillColor(sf::Color::Red);
    else
        m_render_view.setFillColor(sf::Color::Cyan);
}

void Player::move(Move direction)
{
    if(!m_other)
    {
        switch(direction)
        {
            case Up:
                m_render_view.move(sf::Vector2f(0.f, -player_velocity));
                break;
            case Left:
                m_render_view.move(sf::Vector2f(-player_velocity, 0.f));
                break;
            case Down:
                m_render_view.move(sf::Vector2f(0.f, player_velocity));
                break;
            case Right:
                m_render_view.move(sf::Vector2f(player_velocity, 0.f));
                break;
        }
    }
}

void Player::draw(sf::RenderWindow& window)
{
    m_view.setCenter(sf::Vector2f(std::clamp(m_render_view.getPosition().x,100.f,1180.f), std::clamp(m_render_view.getPosition().y, 75.f, 1385.f)));
    if(!m_other) window.setView(m_view);
    window.draw(m_render_view);
    m_flashlight.setPosition(m_render_view.getPosition());

}
