#pragma once

enum Move{Up, Down, Left, Right};
enum PacketType
{
    PlayerPosition = 1,
    ChatMessage,
    EnemyPosition,
    PlayerConnected,
    AllPlayers,
    PlayerDisconnected,
    FlashlightToggle,
    Shot
};
