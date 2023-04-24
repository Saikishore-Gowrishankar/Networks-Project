#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <string>
#include <map>
#include <set>
#include <vector>

#include "Player.h"
#include "Enemy.h"
#include "Projectile.h"
#include "ChatBox.h"

#include "Candle/RadialLight.hpp"
#include "Candle/LightingArea.hpp"

struct ExplosionAnimation
{
    sf::Sprite animation;
    unsigned current_frame_num = 0;
    sf::Vector2f pos;
    std::string file_path = "Resources/Sprites/wills_pixel_explosions_sample/vertical_explosion/PNG/frame";
    unsigned num_frames;
    bool done = false;
};

class Game
{
public:
    Game(std::string const&);
    virtual ~Game() = default;
    
    void run();

private:
    void process_events();
    void draw_map();
    void check_collision();
    void handle_input();
    void update();
    void draw_connected_players();
    void draw_enemies();
    void draw_projectiles();
    void draw_animation();
    void broadcast_player_position();

    ChatBox chat;
    sf::Font font;

    std::vector<std::string> m_tilemap;
    std::vector<sf::FloatRect> m_walls;

    sf::RectangleShape m_tile_shape;
    sf::Texture m_wall_tile;
    sf::Texture m_empty_tile;
    candle::RadialLight m_flashlight, m_other;
    candle::EdgeVector m_map_edges;
    candle::LightingArea m_fog;

    Player m_player;
    sf::Vector2f m_player_oldpos;
    std::map<unsigned,Player> m_other_players;
    std::map<unsigned,Enemy> m_enemies;
    std::vector<Projectile> m_bullets;
    std::vector<ExplosionAnimation> m_explosions;
    std::set<unsigned> m_connected_ids;

    sf::TcpSocket socket;
    sf::IpAddress ip;

    sf::RenderWindow m_window;
    bool m_update = false;        
};
