#include "ChatBox.h"
#include <iostream>

void ChatBox::handle_input(sf::Event e, sf::TcpSocket& socket, unsigned id)
{
    if(!is_selected) return;

    if(e.text.unicode == Backspace)
    {
        m_text = m_text.substr(0, m_text.size() - 1);
        std::cout << "m_text: " << m_text << std::endl;
    }
    else if(e.text.unicode == Enter && !m_text.empty())
    {
        std::string chat_msg = std::string("[PLAYER ") + std::to_string(id) + "] " + m_text;
        buffer.add(chat_msg);
        m_text = "";

        sf::Packet message;
        message << PacketType::ChatMessage << chat_msg;

        socket.send(message);
    }
    else if(m_text.length() < 53)
    {
        m_text += static_cast<char>(e.text.unicode);
        std::cout << "m_text: " << m_text << std::endl;
    }

    textbox.setString(m_text + "_");
}

void ChatBox::draw(sf::RenderWindow& window, sf::View& view)
{
    sf::RectangleShape rectangle, rectangle2;
    rectangle.setSize(sf::Vector2f(m_texture.getSize().x-30, m_texture.getSize().y-30));
    rectangle2.setSize(sf::Vector2f(1000.f, 10.f));
    rectangle.move(15.f, 15.f);
    rectangle2.move(0.f, 20.f * 43.f);

    sf::Color Color1 = sf::Color::Black, Color2 = sf::Color::Green;
    Color1.a = 128;
    Color2.a = 128;
    rectangle.setFillColor(Color1);
    rectangle2.setFillColor(Color2);

    m_texture.clear(Color2);
    m_texture.draw(rectangle);
    m_texture.draw(rectangle2);

    for(int i = 0; i < buffer.size(); ++i)
    {
        logged_text.setString(buffer[i]);
        m_texture.draw(logged_text);
        logged_text.move(0.f, 25.f);
    }
    logged_text.setPosition(sf::Vector2f(20.f, 20.f));

    m_texture.draw(textbox);
    m_texture.display();

    sf::Sprite sprite(m_texture.getTexture());

    sprite.setOrigin(sprite.getGlobalBounds().width / 2.f, sprite.getGlobalBounds().height / 2.f);
    sprite.setPosition(view.getCenter());
    sprite.setScale(sf::Vector2f(0.1f, 0.1f));

    if(is_selected) window.draw(sprite);


}
