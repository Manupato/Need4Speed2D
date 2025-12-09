#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <sys/socket.h>

#include "../../common/queue.h"
#include "../../common/socket.h"
#include "../../common/thread.h"
#include "../command.h"

#include "client_registry.h"
#include "game_manager.h"
#include "receiver.h"
#include "sender.h"

// Hay un client handler por cliente conectado. Dueño de su propio socket,
// receiver y sender. Administra la cola de salida del sender.

class ClientHandler {
private:
    // El socket es propiedad del handler
    Socket peer;
    const int id_;

    // Todos los client handlers comparten el game manager (monitorea todas las partidas)
    GameManager& game_manager;

    // La cola del sender la administra el handler
    Queue<std::shared_ptr<IEvent>> queue_out;

    Receiver receiver;
    Sender sender;

    // Arranca sin estar en ninguna partida. Durante el transcurso de la conexion,
    // puede crear o unirse a todas las partidas que quiera.
    Game* current_game = nullptr;
    Queue<CommandReceiver>* game_cmd_q = nullptr;
    uint32_t current_lobby_id = 0;

    bool is_started{false};
    void attach_to_game(Game& g, uint32_t lobby_id);

public:
    ClientHandler(Socket&& peer, const int id, GameManager& gm);

    // Arranca Receiver y Sender
    void start();

    // Cierra la queue, shutdown al socket, join receiver y sender
    void join();

    // Devuelve true si ambos hilos estan terminados
    bool is_finished() const;

    // Devuelve nullptr si aun no estamos en partida, eoc el puntero a la queue del gameloop
    Queue<CommandReceiver>* get_queue_gameloop() noexcept;

    void join_lobby(const CommandReceiverJoinLobby& cmd);
    void create_lobby(CommandReceiverCreateLobby& cmd);
    void start_lobby(uint32_t lobby_id);
    void disconnect();

    ClientHandler(const ClientHandler&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;

    ~ClientHandler();
};

#endif  // CLIENT_HANDLER_H
