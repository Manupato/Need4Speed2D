#ifndef COMMAND_H
#define COMMAND_H

#include <cstdint>
#include <string>
#include <vector>

enum class CommandReceiverType : uint8_t {
    Move,
    NewCar,
    CreateLobby,
    JoinLobby,
    StartLobby,
    Disconect,
    BeginRace,
    Upgrade,
    DefiniteDisconect
};

// El CommandReceiver es el comando que va a recibir el gameloop desde el receiver
struct CommandReceiver {
    int client_id;
    CommandReceiverType type;
    uint8_t param;       // Direccion de movimiento, modelo del auto, upgrade
    std::string name{};  // Solo para new car
};

// Estos structs son comandos especificos que van a llegar al receiver pero seran
// enviados al gameloop, sino que sirven para el manejo de partidas. Los unicos
// comandos que van al gameloop son los de tipo CommandReceiver
struct CommandReceiverJoinLobby {
    int client_id;
    CommandReceiverType type;
    uint32_t id_lobby;
    uint8_t model_car;
    std::string name;
};
struct CommandReceiverCreateLobby {
    int client_id;
    CommandReceiverType type;
    uint8_t model_car;
    std::vector<std::string> maps;
    std::string name;
};
struct CommandReceiverStartLobby {
    int client_id;
    CommandReceiverType type;
    uint32_t lobby_id;
};


#endif  // COMMAND_H
