#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <iostream>
#include <list>
#include <utility>

#include <sys/socket.h>

#include "../../common/liberror.h"
#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../../common/thread.h"

#include "client_handler.h"
#include "game_manager.h"


class Acceptor: public Thread {

private:
    // Socket de escucha para aceptar conexiones entrantes
    Socket server_socket;

    GameManager& game_manager;

    // Lista para almacenar los manejadores de clientes activos
    std::list<ClientHandler> clients;

    // ID para el proximo cliente
    int next_id = 1;

    // Elimina clientes que ya terminaron
    void reap();

    // Limpia todos los manejadores de clientes (se llama al finalizar el acceptor)
    void clear();

public:
    // Crea un socket de escucha en el puerto dado
    explicit Acceptor(const char* servname, GameManager& game_manager);

    // Acepta conexiones entrantes y les crea un manejador de cliente
    void run() override;

    // Detiene el acceptor cerrando el socket de escucha
    // Si no lo cierra, puede quedar bloqueado en accept()
    void stop() override;

    ~Acceptor() override = default;
};


#endif  // ACCEPTOR_H
