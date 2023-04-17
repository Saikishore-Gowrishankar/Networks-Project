#include <fstream>
#include <iostream>
#include "Game.h"
#include "Common.h"

Game::Game(std::string const& window_name)
    : m_window(sf::VideoMode(1280,720), window_name.c_str()), m_tile_shape(sf::Vector2f(20,20)), ip(sf::IpAddress::getLocalAddress())
{

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
    m_player.get_player().setPosition(sf::Vector2f(x,y));

    socket.setBlocking(false);

    std::ifstream map_file("maze.txt"/*"Resources/Maps/map.txt"*/);
    std::string input;

    if(!m_wall_tile.loadFromFile("Tiles_pack/Tileset_10.png", sf::IntRect(0,32,16,16))) std::cout << "Error!" << std::endl;
    if(!m_empty_tile.loadFromFile("Tiles_pack/Tileset_10.png", sf::IntRect(10*16,4*16,16,16))) std::cout << "Error!" << std::endl;

    while(map_file >> input) m_tilemap.push_back(input);

    for(int i = 0; i < m_tilemap.size(); ++i)
        for(int j = 0; j < m_tilemap[0].size(); ++j)
            if(m_tilemap[i][j] == '#') m_walls.emplace_back(sf::FloatRect(j*20, i*20, 20, 20));

}

void Game::run()
{	
    while(m_window.isOpen())
    {
        process_events();
        handle_input();
        check_collision();
        draw_map();
        //m_player.draw(m_window);
        update();
        broadcast_player_position();
        draw_connected_players();
        //Hande user input
        m_player.draw(m_window);
        m_window.display();
        m_window.clear();
    }
}

void Game::process_events()
{
    sf::Event event;
    while(m_window.pollEvent(event))
    {
        //Window events
        switch(event.type)
        {
            case sf::Event::Closed: m_window.close(); break;
            case sf::Event::GainedFocus: m_update = true; break;
            case sf::Event::LostFocus:   m_update = false; break;
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
                p.get_player().setPosition(sf::Vector2f(x,y));

                m_other_players.insert({id,std::move(p)});
                m_connected_ids.insert(id);
            }
        }
        /*
        Player p(true);
        p.set_id(id);
        p.get_player().setPosition(sf::Vector2f(x,y));

        m_other_players.push_back(std::move(p));
        */
    }
    else if(static_cast<PacketType>(packet_header) == PacketType::PlayerPosition)
    {

        float x, y;
        unsigned id;

        recv >> id >> x >> y;
        std::cout << "Received player position update from " << id << "\n";
        m_other_players[id].get_player().setPosition(sf::Vector2f(x,y));

    }

}

void Game::draw_connected_players()
{
    std::cout << "Size of connected players: " << m_other_players.size() << std::endl;
    for(auto&& [key, elem] : m_other_players)
    {
        elem.draw(m_window);
    }
}

void Game::broadcast_player_position()
{
    if(m_player_oldpos != m_player.get_player().getPosition())
    {
        sf::Packet send_position;
        send_position << PacketType::PlayerPosition << m_player.get_id() <<
                        m_player.get_player().getPosition().x << m_player.get_player().getPosition().y;
        socket.send(send_position);
    }

}

void Game::handle_input()
{
    m_player_oldpos = m_player.get_player().getPosition();
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
        if(m_player.get_player().getGlobalBounds().intersects(elem))
        {
            m_player.get_player().setPosition(m_player_oldpos);
            return;
        }
    }
}
