game:
	g++ Game.cpp Player.cpp main.cpp -std=c++2a -lsfml-graphics -lsfml-network -lsfml-window -lsfml-system
test:
	g++ test.cc -std=c++2a -lsfml-graphics -lsfml-network -lsfml-window -lsfml-system
server:
	g++ Server.cpp -std=c++2a -lsfml-network -lsfml-system -o Server