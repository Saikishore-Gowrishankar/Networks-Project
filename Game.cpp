#include <fstream>
#include <iostream>

#include "Game.h"
#include "Common.h"

Game::Game(std::string const& window_name)
    : m_window(sf::VideoMode(3020, 1460/*1280,720*/), window_name.c_str()), m_tile_shape(sf::Vector2f(20,20)), ip(sf::IpAddress::getLocalAddress()),
      chat("Resources/Fonts/LiberationMono-Regular.ttf", 25), m_fog(candle::LightingArea::FOG,
                                                                    sf::Vector2f(0.f, 0.f),
                                                                    sf::Vector2f(3020.f, 1460.f))
{
    m_flashlight.setRange(50);
    m_flashlight.setFade(false);

    m_other.setRange(150);
    //m_other.setIntensity(0.4);
    m_other.setColor(sf::Color::Red);
    //m_flashlight.setIntensity(0.8);
    sf::Color c = sf::Color::Black;
    c.a = 190;
    m_fog.setAreaColor(c);
    font.loadFromFile("Resources/Fonts/LiberationMono-Regular.ttf");

    //Limit our window to 60FPS
    m_window.setFramerateLimit(60);
    
    //Connect to socket
    socket.connect(ip, 5000);

    sf::Packet recv;
    socket.receive(recv);

    unsigned packet_header{};
    unsigned id;
    float x{}, y{};
    recv >> packet_header >> id >> x >> y;
    if(static_cast<PacketType>(packet_header) == PacketType::PlayerConnected)
        std::cout << "Player joined the game: " << id << std::endl;
    m_player.set_id(id);
    m_player.get_entity().setPosition(sf::Vector2f(x,y));

    socket.setBlocking(false);

    std::ifstream map_file("Resources/Maps/map.txt");
    std::string input;

    if(!m_wall_tile.loadFromFile("Tiles_pack/Tileset_14.png", sf::IntRect(0,32,16,16))) std::cout << "Error!" << std::endl;
    if(!m_empty_tile.loadFromFile("Tiles_pack/Tileset_14.png", sf::IntRect(10*16,4*16,16,16))) std::cout << "Error!" << std::endl;

    while(map_file >> input) m_tilemap.push_back(input);

    for(int i = 0; i < m_tilemap.size(); ++i)
        for(int j = 0; j < m_tilemap[0].size(); ++j)
            if(m_tilemap[i][j] == '#')
            {
                m_walls.emplace_back(sf::FloatRect(j*20, i*20, 20, 20));
                m_map_edges.emplace_back(sf::Vector2f(j*20.f, i*20.f), sf::Vector2f(j*20.f + 20.f, i*20.f));
                m_map_edges.emplace_back(sf::Vector2f(j*20.f + 20.f, i*20.f), sf::Vector2f(j*20.f + 20.f, i*20.f + 20.f));
                m_map_edges.emplace_back(sf::Vector2f(j*20.f + 20.f, i*20.f + 20.f), sf::Vector2f(j*20.f, i*20.f + 20.f));
                m_map_edges.emplace_back(sf::Vector2f(j*20.f, i*20.f + 20.f), sf::Vector2f(j*20.f, i*20.f));
            }

}

void Game::run()
{

sf::Font font;
font.loadFromFile("Resources/Fonts/LiberationMono-Regular.ttf");
// Create a text
sf::Text text("hello", font);
text.setCharacterSize(50);
text.setFillColor(sf::Color::Red);

m_other.setPosition(sf::Vector2f(100.f, 60.f));
m_other.castLight(m_map_edges.begin(), m_map_edges.end());

    while(m_window.isOpen())
    {
        process_events();
        handle_input();
        check_collision();
        draw_map();
        update();
        broadcast_player_position();
        draw_connected_players();
        //Hande user input
        m_player.draw(m_window);


        m_flashlight.setPosition(m_player.get_entity().getPosition());
        //m_flashlight.castLight(m_map_edges.begin(), m_map_edges.end());
        m_fog.clear();
        m_fog.draw(m_player.get_flashlight());
        m_fog.display();
        m_window.draw(m_fog);
        m_window.draw(m_other);
       // m_window.draw(m_flashlight);

        //text.setPosition();

        //text.setPosition(m_player.get_entity().getPosition());
        //m_window.draw(chat.textbox);

        //m_texture.display();
/*
        sf::Sprite sprite(m_texture.getTexture());

        auto&& view = m_player.get_view();

        sprite.setOrigin(sprite.getGlobalBounds().width / 2.f, sprite.getGlobalBounds().height / 2.f);
        sprite.setPosition(view.getCenter());
        sprite.setScale(sf::Vector2f(0.1f, 0.1f));

        if(chat.get_selected()) m_window.draw(sprite);
        //chat.draw(m_window, m_player.get_view());*/

        chat.draw(m_window, m_player.get_view());

        m_window.display();
        m_window.clear();
    }
}

void Game::process_events()
{
    sf::Event event;
    while(m_window.pollEvent(event))
    {
        m_update = !chat.get_selected();
        switch(event.type)
        {
            case sf::Event::Closed: m_window.close(); break;
            case sf::Event::GainedFocus: m_update = true; break;
            case sf::Event::LostFocus:   m_update = false; break;
            case sf::Event::KeyReleased:
                if(event.key.code == sf::Keyboard::Up)
                {
                    chat.toggle_selected();
                }
                break;
            case sf::Event::TextEntered:
            {
                if(chat.get_selected())
                {
                    std::cout << "Text entered: " << event.text.unicode << std::endl;
                    chat.handle_input(event);
                }
                break;
            }
        }
    }
}

void Game::update()
{

    sf::Packet recv;
    int packet_header{};
    socket.receive(recv);

    recv >> packet_header;

    if(static_cast<PacketType>(packet_header) == PacketType::AllPlayers)
    {
        std::cout << "Updating other players!\n";
        float x, y;
        unsigned id;
        unsigned count;

        recv >> count;

        for(int i = 0; i < count; ++i)
        {
            recv >> id >> x >> y;
            if(id != m_player.get_id() && m_connected_ids.find(id) == m_connected_ids.end())
            {
                Player p(true);
                p.set_id(id);
                p.get_entity().setPosition(sf::Vector2f(x,y));

                m_other_players.insert({id,std::move(p)});
                m_connected_ids.insert(id);
            }
        }
        /*
        Player p(true);
        p.set_id(id);
        p.get_entity().setPosition(sf::Vector2f(x,y));

        m_other_players.push_back(std::move(p));
        */
    }
    else if(static_cast<PacketType>(packet_header) == PacketType::PlayerPosition)
    {

        float x, y;
        unsigned id;

        recv >> id >> x >> y;
        std::cout << "Received player position update from " << id << "\n";
        m_other_players[id].get_entity().setPosition(sf::Vector2f(x,y));

    }
    else if(static_cast<PacketType>(packet_header) == PacketType::PlayerDisconnected)
    {
        unsigned id;
        recv >> id;
        
        m_other_players.erase(id);
        m_connected_ids.erase(id);
    }
    

}

void Game::draw_connected_players()
{
    for(auto&& [key, elem] : m_other_players)
    {
        elem.draw(m_window);
    }
}

void Game::broadcast_player_position()
{
    if(m_player_oldpos != m_player.get_entity().getPosition())
    {
        sf::Packet send_position;
        send_position << PacketType::PlayerPosition << m_player.get_id() <<
                        m_player.get_entity().getPosition().x << m_player.get_entity().getPosition().y;
        socket.send(send_position);
    }
}

void Game::handle_input()
{
    m_player_oldpos = m_player.get_entity().getPosition();
    if(m_update)
    {
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            m_player.move(Up);
            //if(shape2.getLocalBounds().intersects())
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            m_player.move(Left);
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            m_player.move(Down);
        }
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            m_player.move(Right);
        }
    }
}

void Game::draw_map()
{
    for(int i = 0; i < m_tilemap.size(); ++i)
    {
        for(int j = 0; j < m_tilemap[0].size(); ++j)
        {
            m_tile_shape.setPosition(20*j, 20*i);
            if(m_tilemap[i][j] == '#')
                m_tile_shape.setTexture(&m_wall_tile);
            else
                m_tile_shape.setTexture(&m_empty_tile);
                            m_window.draw(m_tile_shape);
        }
    }
}

void Game::check_collision()
{
    for(auto&& elem : m_walls)
    {
        if(m_player.get_entity().getGlobalBounds().intersects(elem))
        {
            m_player.get_entity().setPosition(m_player_oldpos);
            return;
        }
    }
}
