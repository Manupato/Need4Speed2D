#ifndef RECEIVER_H
#define RECEIVER_H

#include <iostream>

#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../../common/thread.h"
#include "../command.h"

#include "server_protocol.h"

class ClientHandler;

class Receiver: public Thread {
private:
    // El socket es referenciado, lo tiene el handler
    Socket& peer;

    int id;

    ClientHandler& client_handler;

    // El receiver pushea comandos a esta cola (gameloop)
    Queue<CommandReceiver>* queue_gameloop = {nullptr};

    ServerProtocol protocol;

    void handle_move_command();
    void handle_upgrade_command();
    void handle_join_lobby();
    void handle_create_lobby();
    void handle_start_lobby();

public:
    Receiver(Socket& peer_socket, const int id, ClientHandler& ch);

    void run() override;

    ~Receiver() override = default;
    Receiver(const Receiver&) = delete;
    Receiver& operator=(const Receiver&) = delete;
};
#endif  // RECEIVER_H
