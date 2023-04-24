#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cmath>

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

    m_other.setRange(50);
    //m_other.setIntensity(0.4);
    m_other.setColor(sf::Color(237, 158, 12));
    //m_flashlight.setIntensity(0.8);
    sf::Color c(0,3,0,190);
    m_fog.setAreaColor(c);
    font.loadFromFile("Resources/Fonts/LiberationMono-Regular.ttf");

    //Limit our window to 60FPS
    m_window.setFramerateLimit(60);
    
    //Connect to socket
    socket.connect(ip, 5001);

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
m_other.setPosition(sf::Vector2f(100.f, 60.f));
m_other.castLight(m_map_edges.begin(), m_map_edges.end());
float v = 1.0f;
int i = 0;

    while(m_window.isOpen())
    {
        process_events();
        handle_input();
        check_collision();
        draw_map();
        update();
        broadcast_player_position();
        draw_connected_players();
        draw_enemies();

        //Hande user input
        m_player.draw(m_window);


        //m_flashlight.setPosition(m_player.get_entity().getPosition());
        //m_flashlight.castLight(m_map_edges.begin(), m_map_edges.end());
        m_fog.clear();
        if(m_player.flashlight_on()) m_fog.draw(m_player.get_flashlight());

        for(auto&& [key, other_player] : m_other_players)
        {
            if(other_player.flashlight_on()) m_fog.draw(other_player.get_flashlight());
        }

        m_fog.display();
        m_window.draw(m_fog);
        m_window.draw(m_other);
        draw_projectiles();
        draw_animation();
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
                else if(event.key.code == sf::Keyboard::F && !chat.get_selected())
                {
                    m_player.toggle_flashlight();
                    sf::Packet ToggleFlashlight;
                    ToggleFlashlight << PacketType::FlashlightToggle << m_player.get_id();
                    socket.send(ToggleFlashlight);
                }
                break;
            case sf::Event::TextEntered:
            {
                if(chat.get_selected())
                {
                    std::cout << "Text entered: " << event.text.unicode << std::endl;
                    chat.handle_input(event, socket, m_player.get_id());
                }
                break;
            }
            case sf::Event::MouseButtonReleased:
            {
                sf::Vector2i pixel_mouse_pos = sf::Mouse::getPosition(m_window);
                sf::Vector2f mouse_pos = m_window.mapPixelToCoords(pixel_mouse_pos);
                sf::Vector2f aim_dir = mouse_pos - m_player.get_entity().getPosition();
                sf::Vector2f aim_dir_norm = aim_dir / sqrtf(pow(aim_dir.x, 2) + pow(aim_dir.y, 2));
                if(event.mouseButton.button == sf::Mouse::Left)
                {
                    Projectile bullet;
                    bullet.m_bullet.setPosition(m_player.get_entity().getPosition());
                    bullet.m_current_velocity = aim_dir_norm * 5.f;

                    m_bullets.push_back(std::move(bullet));

                }
                break;
            }
        }
    }
}

void Game::update()
{

    for(auto&& b : m_bullets)
    {
        b.m_old_pos = b.m_bullet.getPosition();
        b.move(b.m_current_velocity);
    }

    sf::Packet recv;
    int packet_header{};
    socket.receive(recv);

    recv >> packet_header;

    if(static_cast<PacketType>(packet_header) == PacketType::AllPlayers)
    {
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
    else if(static_cast<PacketType>(packet_header) == PacketType::ChatMessage)
    {
        std::string message;
        recv >> message;

        chat.get_buffer().add(message);
    }
    else if(static_cast<PacketType>(packet_header) == PacketType::FlashlightToggle)
    {
        unsigned id;
        recv >> id;
        m_other_players[id].toggle_flashlight();
    }
    else if(static_cast<PacketType>(packet_header) == PacketType::EnemyPosition)
    {
        std::cout << "GETTNG ENEMI" << std::endl;
        unsigned id, count;
        float x, y;

        recv >> count;

        for(int i = 0; i < count; ++i)
        {
            recv >> id >> x >> y;
            std::cout << "ID: " << id << "at " << x << " " << y << std::endl;
            m_enemies[id].set_id(id);
            m_enemies[id].posx = x; m_enemies[id].posy = y;//m_monster.setPosition(sf::Vector2f(x,y));
        }
    }
    

}

void Game::draw_connected_players()
{
    for(auto&& [key, elem] : m_other_players)
    {
        elem.draw(m_window);
    }
}

void Game::draw_enemies()
{
    //std::cout << "Drawing enemies: " << m_enemies.size() << std::endl;
    for(auto&& [key, elem] : m_enemies)
    {
        elem.draw(m_window);
    }
}

void Game::draw_projectiles()
{
    for(auto&& b : m_bullets)
    {
        b.draw(m_window);
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
    if(chat.get_selected()) return;
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

void Game::draw_animation()
{
    int count = 0;
    for(int i = 0; i < m_explosions.size(); ++i)
    {
        sf::Texture texture;
        std::ostringstream ss;

        if(m_explosions[i].current_frame_num == m_explosions[i].num_frames)
        {
            m_explosions[i].done = true;
            continue;
        }
        ss << std::setfill('0') << std::setw(4) << m_explosions[i].current_frame_num;
        ss << ".png";
        std::cout << m_explosions[i].file_path + ss.str() << std::endl;
        texture.loadFromFile(m_explosions[i].file_path + ss.str());

        m_explosions[i].animation.setTexture(texture);
        m_explosions[i].animation.setOrigin(m_explosions[i].animation.getGlobalBounds().width/2.f, m_explosions[i].animation.getGlobalBounds().height/2.f);
        m_explosions[i].animation.setPosition(m_explosions[i].pos);
        m_window.draw(m_explosions[i].animation);

        m_explosions[i].current_frame_num++;
            //m_explosions.erase(std::cbegin(m_explosions) + count++);
    }
    for(int i = 0; i < m_explosions.size(); ++i) if(m_explosions[i].done) m_explosions.erase(std::cbegin(m_explosions) + i);

}

void Game::check_collision()
{
    for(auto&& wall : m_walls)
    {
        if(m_player.get_entity().getGlobalBounds().intersects(wall))
        {
            m_player.get_entity().setPosition(m_player_oldpos);
            break;
        }
    }
    for(auto&& [id, enemy] : m_enemies)
    {
        if(m_player.get_entity().getGlobalBounds().intersects(enemy.m_monster.getGlobalBounds()))
        {
            m_player.get_entity().setPosition(m_player_oldpos);
            break;
        }
    }
    for(int i = 0; i < m_bullets.size(); ++i)
    {
        for(auto&& wall : m_walls)
            if(m_bullets[i].m_bullet.getGlobalBounds().intersects(wall))
            {
                ExplosionAnimation e;
                e.file_path = "Resources/Sprites/wills_pixel_explosions_sample/X_plosion/PNG/frame";
                e.num_frames = 64;
                e.pos = m_bullets[i].m_old_pos;
                m_explosions.push_back(std::move(e));
                m_bullets.erase(std::cbegin(m_bullets) + i);
                break;
            }
    }
    for(int i = 0; i < m_bullets.size(); ++i)
    {
        for(auto&& [id, enemy] : m_enemies)
        {
            if(m_bullets[i].m_bullet.getGlobalBounds().intersects(enemy.m_monster.getGlobalBounds()))
            {
                ExplosionAnimation e;
                e.file_path = "Resources/Sprites/wills_pixel_explosions_sample/vertical_explosion_small/PNG/frame";
                e.num_frames = 65;
                e.pos = m_bullets[i].m_old_pos;
                m_explosions.push_back(std::move(e));
                m_bullets.erase(std::cbegin(m_bullets) + i);
                if(--enemy.m_health == 0)
                {
                    ExplosionAnimation e;
                    e.file_path = "Resources/Sprites/wills_pixel_explosions_sample/vertical_explosion/PNG/frame";
                    e.num_frames = 74;
                    e.pos = m_bullets[i].m_old_pos;
                    m_explosions.push_back(std::move(e));
                    m_enemies.erase(id);
                }
                break;
            }
        }
    }
}
