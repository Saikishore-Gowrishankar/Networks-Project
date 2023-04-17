#include <algorithm>
#include "Player.h"
#include "Common.h"
Player::Player(bool other)
    : m_view(sf::FloatRect(0,0,200,150)), m_other(other)
{
    m_player_view.setRadius(5);

    if(other)
        m_player_view.setFillColor(sf::Color::Red);
    else
        m_player_view.setFillColor(sf::Color::Cyan);
}

void Player::move(Move direction)
{
    if(!m_other)
    {
        switch(direction)
        {
            case Up:
                m_player_view.move(sf::Vector2f(0.f, -player_velocity));
                break;
            case Left:
                m_player_view.move(sf::Vector2f(-player_velocity, 0.f));
                break;
            case Down:
                m_player_view.move(sf::Vector2f(0.f, player_velocity));
                break;
            case Right:
                m_player_view.move(sf::Vector2f(player_velocity, 0.f));
                break;
        }
    }
}

void Player::draw(sf::RenderWindow& window)
{
    m_view.setCenter(sf::Vector2f(std::clamp(m_player_view.getPosition().x,100.f,1180.f), std::clamp(m_player_view.getPosition().y, 75.f, 645.f)));
    if(!m_other) window.setView(m_view);
    window.draw(m_player_view);
}
