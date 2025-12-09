#ifndef SENDER_H
#define SENDER_H

#include <cstdint>
#include <iostream>
#include <memory>

#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../../common/thread.h"
#include "../event.h"

#include "server_protocol.h"

// El sender solamente saca eventos de la cola y los manda por el socket
// Cada client handler tiene su propio sender y su propia cola
// El receiver en cambio, comparte la cola de comandos del gameloop con
// todos los receivers de los clientes en la misma partida.

class Sender: public Thread {
private:
    // El socket es referenciado, lo tiene el handler
    Socket& peer;

    const int id_;

    // La cola es prioridad del client_handler. Es el mismo quien la agrega/saca del
    // registry y tambien le pushea logica de la lobby.
    Queue<std::shared_ptr<IEvent>>& queue_out;

    ServerProtocol protocol;

public:
    Sender(Socket& peer_socket, const int id, Queue<std::shared_ptr<IEvent>>& queue_out);

    void run() override;

    void close_queue();

    ~Sender() override = default;
    Sender(const Sender&) = delete;
    Sender& operator=(const Sender&) = delete;
};
#endif  // SENDER_H
