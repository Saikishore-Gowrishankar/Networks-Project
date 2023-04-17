#pragma once

enum Move{Up, Down, Left, Right};
enum PacketType
{
    PlayerPosition = 1,
    ChatMessage = 2,
    EnemyPosition = 3,
    PlayerConnected = 4,
    AllPlayers = 5
};
