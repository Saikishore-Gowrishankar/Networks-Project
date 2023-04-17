#include <SFML/Network.hpp>
#include <vector>
#include <memory>
#include <iostream>
#include "Common.h"

namespace
{
    struct client_data
    {
        int player_ID;
        sf::Vector2f position;
    };
    std::vector<client_data> connected_players;
}

int main()
{
    // Create a socket to listen to new connections
    sf::TcpListener listener;
    listener.listen(5000);
    std::vector<std::unique_ptr<sf::TcpSocket>> clients;
    static unsigned client_ID = 0;

    // Create a selector
    sf::SocketSelector selector;
    selector.add(listener);

    // Endless loop that waits for new connections
    while (true)
    {
        std::cout << "Waiting on selector\n";
        // Make the selector wait for data on any socket
        if (selector.wait())
        {
            std::cout << "Is listener ready?\n";
            if (selector.isReady(listener))
            {
                // The listener is ready: there is a pending connection
                auto client = std::make_unique<sf::TcpSocket>();
                if (listener.accept(*client) == sf::Socket::Done)
                {
                    std::cout << "Accepted a new client " << client_ID << std::endl;
                    sf::Packet init;

                    float x, y;
                    if (client_ID == 0) x = y = 20.f;
                    else x = y = 20.f;

                    connected_players.emplace_back(client_ID, sf::Vector2f(x,y));

                    init << PacketType::PlayerConnected << client_ID++ << x <<  y;

                    client->setBlocking(false);

                    client->send(init);

                    selector.add(*client);

                    // Add the new client to the clients list
                    clients.push_back(std::move(client));

                    sf::Packet broadcast;
                    broadcast << PacketType::AllPlayers << (sf::Uint32) connected_players.size();
                    for(auto&& it : connected_players) broadcast << it.player_ID << it.position.x << it.position.y;
                   //Broadcast all connection to all clients
                    for (auto&& it : clients)
                    {
                        static int count = 0;
                        std::cout << "Broadcasting to " << count++;
                        it->send(broadcast);
                    }

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
                std::cout << "Socket is now testing clients\n";
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
                    }
                }
            }
        }
    }
}
