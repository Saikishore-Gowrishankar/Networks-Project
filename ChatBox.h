#pragma once
#include <SFML/Graphics.hpp>
#include <sstream>
#include <memory>
#include <iostream>
#include "ChatBuffer.h"

enum Keys
{
    Backspace = 8,
    Enter = 13,
    Escape = 27
};

class ChatBox
{
public:
    //ChatBox() { textbox.setFillColor(sf::Color::Red); }
    explicit ChatBox(std::string const& font_filename, size_t size)
        : m_font(std::make_unique<sf::Font>()), m_text(""), m_size(size)
    {

        if(m_font->loadFromFile(font_filename))
        {
            std::cout << "Font loaded" << std::endl;
            textbox.setFont(*m_font);
            textbox.setCharacterSize(size);
            textbox.setFillColor(sf::Color::Green);

            logged_text = textbox;
            logged_text.move(20.f, 20.f);

            textbox.setString("_");
            textbox.move(20.f, 20.f * 45.f);

            m_texture.create(1000,1000);
        }
        else
        {
            std::cout << "ERROR!!!!" << std::endl;
        }
    }

    void handle_input(sf::Event);
    void draw(sf::RenderWindow&, sf::View&);

    void toggle_selected() { is_selected = !is_selected; }
    bool get_selected() const { return is_selected; }

private:
    ChatBuffer buffer;
    sf::Text textbox, logged_text;
    std::unique_ptr<sf::Font> m_font;

    sf::RenderTexture m_texture;
    std::string m_text;
    size_t m_size;
    bool is_selected = false;
};

