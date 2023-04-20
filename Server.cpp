#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <iterator>
#include <memory>
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
        for(auto&& enemy : enemies)
            enemy.move(enemy.get_travel_direction());
    }

    template<typename T>
    void broadcast_enemy_positions(T const& clients)
    {
    }

}

int main()
{
    std::srand(time(NULL));

    // Create a socket to listen to new connections
    sf::TcpListener listener;
    listener.listen(5000);
    std::vector<std::unique_ptr<sf::TcpSocket>> clients;
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

    for(int i = 0; i < 2; ++ i)
    {
        unsigned index = rand() % empty_spaces.size();
        auto&& start_pos = empty_spaces[index];
        //Enemy e(i, start_pos.x, start_pos.y);
        
        enemies.emplace_back(Enemy(i, start_pos.x, start_pos.y));

        empty_spaces.erase(std::cbegin(empty_spaces) + index);
    }


    // Endless loop that waits for new connections
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

                    init << PacketType::PlayerConnected << client_ID++ << start_pos.x <<  start_pos.y;

                    client->setBlocking(false);

                    client->send(init);

                    selector.add(*client);

                    // Add the new client to the clients list
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
                    std::cout << "Broadcasted init packet for client #" << client_ID - 1  << std::endl;
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
                            packet >> packet_header;
                            PacketType Header = static_cast<PacketType>(packet_header);

                            switch(Header)
                            {
                                case PacketType::PlayerPosition:
                                    packet >> id >> x >> y;
                                    connected_players[id].position = sf::Vector2f(x,y);
                                    send_packet << PacketType::PlayerPosition << id << x  << y;

                                    for(int j = 0; j < clients.size(); ++j)
                                        if(i != j) clients[j]->send(send_packet);
                                    break;
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
        update_enemy_positions();
        //broadcast_to_clients(clients, EnemyPositions);

        //sf::Packet p; p << PacketType::EnemyMovement;
        //broadcast_to_clients(clients, p);
        //broadcast_enemy_positions(clients);
    }
}
