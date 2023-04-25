#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <memory>
#include <thread>
#include <mutex>
#include <iostream>
#include "Common.h"
#include "Enemy.h"
#include "Projectile.h"

#define thread_safe_cout(s) { cout_lock.lock(); std::cout << s << std::endl; cout_lock.unlock(); }

namespace
{
    struct client_data
    {
        int player_ID;
        sf::Vector2f position;
    };
    std::vector<client_data> connected_players;
    std::vector<std::string> map;
    std::vector<sf::Vector2f> empty_spaces;
    std::vector<Enemy> enemies;
    std::vector<std::unique_ptr<sf::TcpSocket>> clients;

    std::mutex enemy_lock;
    std::mutex client_lock;
    std::mutex cout_lock;

    template<typename T>
    void broadcast_to_clients(T const& clients, sf::Packet& broadcast)
    {
        for(auto&& it : clients) it->send(broadcast);
    }

    template<typename T>
    void broadcast_enemy_positions(T const& clients)
    {
        if(true)
        {
            enemy_lock.lock();
            sf::Packet EnemyPositions;
            EnemyPositions << PacketType::EnemyPosition << (sf::Uint32) enemies.size();

            for(auto&& e : enemies) EnemyPositions << e.get_id() << e.get_entity().getPosition().x << e.get_entity().getPosition().y;
            enemy_lock.unlock();


            client_lock.lock();
            broadcast_to_clients(clients, EnemyPositions);
            client_lock.unlock();
        }
    }

    void accept_debug_input()
    {
        std::string input;
        while(true)
        {
            std::cin >> input;
            thread_safe_cout("Debug: " << input);
        }
    }

}

int main()
{
    std::srand(time(NULL));

    int counter = 0;

    // Create a socket to listen to new connections
    sf::TcpListener listener;
    listener.listen(5001);

    static unsigned client_ID = 0;

    // Create a selector
    sf::SocketSelector selector;
    selector.add(listener);

    std::ifstream map_stream("Resources/Maps/map.txt");
    std::string row;
    while(map_stream >> row) map.push_back(row);

    for(int i = 0; i < map.size(); ++i)
    {
        for(int j = 0; j < map[0].size(); ++j)
        {
            if(map[i][j] != '#')
            {
                empty_spaces.push_back(sf::Vector2f(j*20, i*20));
            }
        }
    }

    for(int i = 0; i < 250; ++ i)
    {
        unsigned index = rand() % empty_spaces.size();
        auto&& start_pos = empty_spaces[index];
        //Enemy e(i, start_pos.x, start_pos.y);

        Enemy e;
        e.set_id(i);
        e.get_entity().setPosition(sf::Vector2f(start_pos.x,start_pos.y));
        enemies.push_back(e);

        empty_spaces.erase(std::cbegin(empty_spaces) + index);
    }

    unsigned tileset_number = rand() % 14 + 1;

    std::thread t{accept_debug_input};
    t.detach();
    
    thread_safe_cout("Server IP: "  << sf::IpAddress::getLocalAddress().toString());
    while (true)
    {
        // Make the selector wait for data on any socket
        if (selector.wait())
        {
            if (selector.isReady(listener))
            {
                // The listener is ready: there is a pending connection
                auto client = std::make_unique<sf::TcpSocket>();
                if (listener.accept(*client) == sf::Socket::Done)
                {
                    thread_safe_cout("Accepted a new client " << client_ID);
                    sf::Packet init;

                    auto&& start_pos = empty_spaces[rand() % 10];

                    connected_players.emplace_back(client_ID, start_pos);

                    init << PacketType::PlayerConnected << client_ID++ << start_pos.x <<  start_pos.y << tileset_number;

                    client->setBlocking(false);

                    client->send(init);

                    selector.add(*client);

                    // Add the new client to the clients list
                    client_lock.lock();
                    clients.push_back(std::move(client));

                    sf::Packet broadcast;
                    broadcast << PacketType::AllPlayers << (sf::Uint32) connected_players.size();
                    for(auto&& it : connected_players) broadcast << it.player_ID << it.position.x << it.position.y;

                    broadcast_to_clients(clients, broadcast);
                    client_lock.unlock();

                    thread_safe_cout("Broadcasted init packet for client #" << client_ID - 1);

                    broadcast_enemy_positions(clients);

                }
                else
                {
                    // Error, we won't get a new connection, delete the socket
                    client.reset(nullptr);
                }
            }
            else
            {
                // The listener socket is not ready, test all other sockets (the clients)
                for (int i = 0; i < clients.size(); ++i)
                {
                    if (selector.isReady(*clients[i]))
                    {
                        // The client has sent some data, we can receive it
                        sf::Packet packet, send_packet;
                        if (clients[i]->receive(packet) == sf::Socket::Done)
                        {
                            unsigned packet_header, id;
                            float x, y;
                            std::string message;
                            packet >> packet_header;
                            PacketType Header = static_cast<PacketType>(packet_header);

                            switch(Header)
                            {
                                case PacketType::PlayerPosition:
                                    packet >> id >> x >> y;
                                    connected_players[id].position = sf::Vector2f(x,y);
                                    send_packet << PacketType::PlayerPosition << id << x  << y;
                                    for(int j = 0; j < clients.size(); ++j) if(i != j) clients[j]->send(send_packet);
                                    break;
                                case PacketType::ChatMessage:
                                    packet >> message;
                                    send_packet << PacketType::ChatMessage << message;
                                    for(int j = 0; j < clients.size(); ++j) if(i != j) clients[j]->send(send_packet);
                                    break;
                                case PacketType::FlashlightToggle:
                                    packet >> id;
                                    send_packet << PacketType::FlashlightToggle << id;
                                    for(int j = 0; j < clients.size(); ++j) if(i != j) clients[j]->send(send_packet);
                                    break;
                                case PacketType::Shot:
                                    packet >> x >> y;
                                    send_packet << PacketType::Shot << x << y;
                                    packet >> x >> y;
                                    send_packet << x << y;
                                    for(int j = 0; j < clients.size(); ++j) if(i != j) clients[j]->send(send_packet);

                            }
                        }
                        else
                        {
                            thread_safe_cout("Client disconnected: " << i);
                            selector.remove(*clients[i]);
                            clients.erase(clients.cbegin() + i);
                            
                            sf::Packet broadcast;
                            broadcast << PacketType::PlayerDisconnected << i;
                            broadcast_to_clients(clients, broadcast);
                        }
                    }
                }
            }
        }
    }
}
