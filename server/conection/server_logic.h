#ifndef SERVER_LOGIC_H
#define SERVER_LOGIC_H

#include <iostream>

#include "acceptor.h"
#include "game_manager.h"

class ServerLogic {
private:
    // Monitor que administra las games del juego
    GameManager game_manager;

    // Acepta conexiones entrantes
    Acceptor acceptor;

public:
    explicit ServerLogic(const char* service_or_port);

    // Solo espera a que el usuario presione 'q'
    int run();

    ~ServerLogic();
    ServerLogic(const ServerLogic& other) = delete;
    ServerLogic& operator=(const ServerLogic& other) = delete;
};

#endif  // SERVER_LOGIC_H
