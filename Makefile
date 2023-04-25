game:
	g++ ChatBox.cpp Game.cpp Player.cpp main.cpp -std=c++2a -IResources/Libraries/include -LResources/Libraries -lCandle-s -lsfml-graphics -lsfml-network -lsfml-window -lsfml-system -lpthread
test:
	g++ test.cc -std=c++2a -lsfml-graphics -lsfml-network -lsfml-window -lsfml-system -lpthread
server:
	g++ Server.cpp -std=c++2a -lsfml-graphics -lsfml-network -lsfml-system -lpthread -o Server
all:
	make game
	make server
	./Server
