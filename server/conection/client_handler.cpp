#include "client_handler.h"

ClientHandler::ClientHandler(Socket&& peer, const int id, GameManager& gm):
        peer(std::move(peer)),
        id_(id),
        game_manager(gm),
        receiver(this->peer, this->id_, *this),
        sender(this->peer, this->id_, queue_out) {}

void ClientHandler::start() {
    if (!is_started) {
        is_started = true;
        sender.start();
        receiver.start();
    }
}

void ClientHandler::join() {
    if (!is_started)
        return;

    if (current_game) {
        current_game->remove_lobby_player(id_);
        current_game->get_registry().remove(id_);

        // Si la lobby aun no empezo, avisamos al resto q nos fuimos
        if (!current_game->is_started()) {
            LobbySnapshotData snap_data;
            current_game->build_lobby_snapshot(current_lobby_id, snap_data);

            auto ev = std::make_shared<LobbySnapshotEvent>(std::move(snap_data));
            current_game->get_registry().broadcast(ev);
        }

        current_game = nullptr;
        game_cmd_q = nullptr;
        current_lobby_id = 0;
    }

    sender.close_queue();

    try {
        peer.shutdown(SHUT_RDWR);
    } catch (...) {}
    receiver.join();
    sender.join();
    is_started = false;
}

// Si alguno dejo de estar vivo, el handler ya termino y debe hacer join de ambos
bool ClientHandler::is_finished() const { return !receiver.is_alive() && !sender.is_alive(); }

Queue<CommandReceiver>* ClientHandler::get_queue_gameloop() noexcept {
    if (current_game && current_game->has_finished()) {
        current_game = nullptr;
        game_cmd_q = nullptr;
        current_lobby_id = 0;
        return nullptr;
    }
    if (current_game && current_game->is_started()) {
        game_cmd_q = &current_game->get_cmd_q();
    }
    return game_cmd_q;
}

void ClientHandler::attach_to_game(Game& g, uint32_t lobby_id) {
    current_game = &g;
    current_lobby_id = lobby_id;
    // Hasta que no inicie el juego realmente (salir de la lobby), no le asignamos la cola
}

void ClientHandler::create_lobby(CommandReceiverCreateLobby& cmd) {
    Game* g = NULL;
    LobbySnapshotData snapshot;

    bool ok = game_manager.create_lobby_and_join(cmd.client_id, cmd.model_car, queue_out, g,
                                                 snapshot, cmd.maps, cmd.name);


    if (!ok) {
        auto error = std::make_shared<JoinErrorEvent>();
        queue_out.push(error);
        return;
    }
    attach_to_game(*g, snapshot.lobby_id);
}

void ClientHandler::join_lobby(const CommandReceiverJoinLobby& cmd) {
    Game* g = NULL;
    LobbySnapshotData snapshot;

    bool ok = game_manager.join_lobby(cmd.client_id, cmd.id_lobby, cmd.model_car, queue_out, g,
                                      snapshot, cmd.name);
    if (!ok) {
        auto error = std::make_shared<JoinErrorEvent>();
        queue_out.push(error);
        return;
    }
    attach_to_game(*g, snapshot.lobby_id);
}

void ClientHandler::start_lobby(uint32_t lobby_id) {
    if (!current_game)
        return;
    if (game_manager.start_lobby(lobby_id)) {
        game_cmd_q = &current_game->get_cmd_q();
    }
}

void ClientHandler::disconnect() {
    game_manager.disconnect(id_);

    current_game = nullptr;
    game_cmd_q = nullptr;
    current_lobby_id = 0;
}

ClientHandler::~ClientHandler() { join(); }
