#pragma once
#include <SFML/Network.hpp>
#include "Common.h"

class Client
{
public:
    sf::TcpSocket* socket;     // TCP socket for communication
    sf::Packet packet;        // Packet for sending/receiving data
    bool isConnected;        // Flag to indicate if client is connected

    // Constructor
    Client()
    {
        isConnected = false;
    }

    // Send chat message to server
    void sendChatMessage(const std::string& message)
    {
        // Serialize chat message to packet
        packet << ChatMessage << message;

        // Send chat packet to server
        if (socket->send(packet) != sf::Socket::Done)
        {
            // Handle send error
        }
    }

    // Send player position to server
    void sendPlayerPosition(const sf::Vector2f& position)
    {
        // Serialize player position to packet
        packet << PlayerPosition << position.x << position.y;

        // Send position packet to server
        if (socket->send(packet) != sf::Socket::Done)
        {
            // Handle send error
        }
    }

    // Receive game state updates from server
    void receivePlayerPosition()
    {
        // Receive game state packet from server
        if (socket->receive(packet) == sf::Socket::Done)
        {
            // Deserialize game state from packet
            // Update local game state with received data
        }
    }
};
