#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
int main()
{
    sf::IpAddress ip = sf::IpAddress::getLocalAddress();
    sf::TcpSocket socket;
    char connectionType;
    
    std::cout << "(s) for server, c for client" << std::endl;
    std::cin >> connectionType;
    
    if(connectionType == 's')
    {
        sf::TcpListener listener;
        listener.listen(5000);
        if(listener.accept(socket) == sf::Socket::Done)
        {
            std::cout << "awsdsfConnected to :" << socket.getRemoteAddress() << std::endl;
        }
        std::cout << "end" << std::endl;
    }
    else socket.connect(ip, 5000);
    std::cout << "Aftersies" << std::endl;
    srand(time(NULL));
    sf::RenderWindow window(sf::VideoMode(1280, 720), "SFML App");
    window.setFramerateLimit(60);
    sf::RectangleShape shape(sf::Vector2f(20,20));
    sf::CircleShape shape2, shape3;
    shape2.setRadius(5);
    shape3.setRadius(5);
    shape2.setFillColor(sf::Color::Cyan);
    shape3.setFillColor(sf::Color::Red);
    sf::Clock clock;
    sf::Vector2f movement(10.f,10.f);
    sf::Time deltaTime = clock.restart();
    unsigned char a = rand() % 256, b = rand() % 256, c = rand() % 256;
    
    sf::View view(sf::FloatRect(0,0,200,150));

    std::vector<std::string> map;
    std::vector<sf::FloatRect> walls;
    std::ifstream map_file("map.txt"), texture_file("Tiles_pack/Tileset_1.png");
    std::string input;
    sf::Texture tile;
    
    if(!tile.loadFromFile("Tiles_pack/Tileset_10.png", sf::IntRect(0,32,16,16))) std::cout << "Error!" << std::endl;
    
    while(map_file >> input) map.push_back(input);
    
    for(int i = 0; i < map.size(); ++i)
        for(int j = 0; j < map[0].size(); ++j)
            if(map[i][j] == '#') walls.emplace_back(sf::FloatRect(j*20, i*20, 20, 20));


    socket.setBlocking(false);
    sf::Vector2f prevPosition, p2Position;
    bool update = false;
    while(window.isOpen())
    {
        sf::Event event;
        while(window.pollEvent(event))
        {
            if(event.type == sf::Event::Closed) window.close();
            else if(event.type == sf::Event::GainedFocus) update = true;
            else if(event.type == sf::Event::LostFocus) update = false;
        }
        
        prevPosition = shape2.getPosition();
        int x = std::floor(shape2.getPosition().x/20),
                 y = std::floor(shape2.getPosition().y/20);

        if(update)
        {
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::W)) 
            {
                shape2.move(sf::Vector2f(0.f, -2.f));
                //if(shape2.getLocalBounds().intersects())
            }
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::A)) shape2.move(sf::Vector2f(-2.f, 0.f));
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::S)) shape2.move(sf::Vector2f(0.f, 2.f));
            if(sf::Keyboard::isKeyPressed(sf::Keyboard::D)) shape2.move(sf::Vector2f(2.f, 0.f));
            for(auto&& elem : walls)
                if(shape2.getGlobalBounds().intersects(elem))
                    shape2.setPosition(prevPosition);

        }

        sf::Packet packet;
        if(prevPosition != shape2.getPosition())
        {
            packet << shape2.getPosition().x << shape2.getPosition().y;
            socket.send(packet);
        }
        
        socket.receive(packet);
        if(packet >> p2Position.x >> p2Position.y)
        {
            shape3.setPosition(p2Position);
        }
        
        for(int i = 0; i < map.size(); ++i)
        {
            for(int j = 0; j < map[0].size(); ++j)
            {
                if(map[i][j] == '#')
                {
                    shape.setPosition(20*j, 20*i);
                   // shape.setFillColor(sf::Color(a,b,c));
                   shape.setTexture(&tile);
                    window.draw(shape);
                }
            }
        }
        view.setCenter(sf::Vector2f(std::clamp(shape2.getPosition().x,100.f,1180.f), std::clamp(shape2.getPosition().y, 75.f, 645.f)));
        window.draw(shape2);
        window.draw(shape3);
        window.setView(view);
        window.display();
        window.clear();

        //std::cout << clock.getElapsedTime().asSeconds() << std::endl;
    }
}