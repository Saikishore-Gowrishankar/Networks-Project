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
        for(auto&& it : clients)
        {
            it->send(broadcast);
        }
    }

    void update_enemy_positions()
    {
        enemy_lock.lock();
        for(auto&& enemy : enemies)
            enemy.move(enemy.get_travel_direction());
        enemy_lock.unlock();
    }

    template<typename T>
    void broadcast_enemy_positions(T const& clients)
    {
        if(true)//clock.getElapsedTime().asMilliseconds() >= 100)
        {
           // std::cout << "100ms!\n";
            enemy_lock.lock();
            sf::Packet EnemyPositions;
            EnemyPositions << PacketType::EnemyPosition << (sf::Uint32) enemies.size();

            for(auto&& e : enemies)
            {
                EnemyPositions << e.get_id() << e.get_entity().getPosition().x << e.get_entity().getPosition().y;
            std::cout << "ID: " << e.get_id() << "at " << e.get_entity().getPosition().x << " " << e.get_entity().getPosition().y << std::endl;
            }
            enemy_lock.unlock();


            client_lock.lock();
            broadcast_to_clients(clients, EnemyPositions);
            client_lock.unlock();
            //clock.restart();
        }
    }

    void send_enemy_update()
    {
        while(true)
        {
            //std::cout << "Sending update!\n";
            update_enemy_positions();
            broadcast_enemy_positions(clients);
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

        std::cout << "Spawning enemy " << e.get_id() << " at: " << e.get_entity().getPosition().x << " " << e.get_entity().getPosition().y;
        enemies.push_back(e);

        empty_spaces.erase(std::cbegin(empty_spaces) + index);
    }

    unsigned tileset_number = rand() % 14 + 1;

    // Endless loop that waits for new connections
    //sf::Clock clock;
    //std::thread t{send_enemy_update};
    //t.detach();
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
                    std::cout << "Accepted a new client " << client_ID << std::endl;
                    sf::Packet init;

                    float x, y;
                    if (client_ID == 0) x = 20.f * 20, y = 2.f * 20;//x = y = 20.f;
                    else x = y = 20.f;

                    auto&& start_pos = /*sf::Vector2f(x,y);*/ empty_spaces[rand() % 10];

                    connected_players.emplace_back(client_ID, start_pos/*sf::Vector2f(x,y)*/);

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
                   //Broadcast all connection to all clients
                 /*
                    for (auto&& it : clients)
                    {
                        static int count = 0;
                        std::cout << "Broadcasting to " << count++;
                        it->send(broadcast);
                    }
*/
                    broadcast_to_clients(clients, broadcast);
                    client_lock.unlock();
                    std::cout << "Broadcasted init packet for client #" << client_ID - 1  << std::endl;

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

                            }
                        }
                        else
                        {
                            std::cout << "Client disconnected: " << i << std::endl;
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

        //Other stuff?
        //std::cout << "Doing other stuffs!!" << counter ++ << std::endl;

        //sf::Packet p; p << PacketType::EnemyMovement;
        //broadcast_to_clients(clients, p);
        //broadcast_enemy_positions(clients);
    }
}
