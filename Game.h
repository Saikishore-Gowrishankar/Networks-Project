#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <string>
#include <map>
#include <set>

#include "Player.h"

class Game
{
public:
    Game(std::string const&);
    virtual ~Game() = default;
    
    void run();
    void process_events();
    void draw_map();
    void check_collision();
    void handle_input();
    void update();
    void draw_connected_players();
    void broadcast_player_position();
private:
    std::vector<std::string> m_tilemap;
    std::vector<sf::FloatRect> m_walls;

    sf::RectangleShape m_tile_shape;
    sf::Texture m_wall_tile;
    sf::Texture m_empty_tile;

    Player m_player;
    sf::Vector2f m_player_oldpos;
    std::map<unsigned,Player> m_other_players;
    std::set<unsigned> m_connected_ids;

    sf::TcpSocket socket;
    sf::IpAddress ip;

    sf::RenderWindow m_window;
    bool m_update = false;        
};
