#include "game.h"

#include <string>
#include <vector>


Game::Game(std::vector<std::string>& maps, int lobby_id):
        command_queue(), registry(), gameloop(command_queue, registry, maps), lobby_id(lobby_id) {}

void Game::start() { gameloop.start(); }

void Game::stop() {
    try {
        command_queue.close();
    } catch (...) {}
    gameloop.stop();
}

void Game::join() { gameloop.join(); }

Queue<CommandReceiver>& Game::get_cmd_q() { return command_queue; }
ClientRegistryMonitor& Game::get_registry() { return registry; }

const Queue<CommandReceiver>& Game::get_cmd_q() const { return command_queue; }

const ClientRegistryMonitor& Game::get_registry() const { return registry; }

bool Game::start_lobby() {
    if (lobby_started)
        return false;
    lobby_started = true;

    // Crear autos para todos los jugadores de la lobby
    // Recien cuando se arranquen a crear autos, se empezaria a mandar snapshots
    for (const auto& [player_id, model]: lobby_players) {
        CommandReceiver cmd;
        cmd.client_id = player_id;
        cmd.type = CommandReceiverType::NewCar;
        cmd.param = model;
        cmd.name = names.at(player_id);
        command_queue.push(cmd);
    }

    CommandReceiver begin;
    begin.client_id = -1;  // interno
    begin.type = CommandReceiverType::BeginRace;
    begin.param = 0;
    command_queue.push(begin);

    return true;
}

void Game::build_lobby_snapshot(uint32_t lobby_id, LobbySnapshotData& out) const {
    out.lobby_id = lobby_id;
    out.lobby_players.clear();

    for (std::map<int, uint8_t>::const_iterator it = lobby_players.begin();
         it != lobby_players.end(); ++it) {
        LobbyPlayer lp;

        auto it_name = names.find(it->first);
        if (it_name != names.end()) {
            lp.name = it_name->second;
        } else {
            lp.name = "Jugador " + std::to_string(it->first);
        }
        lp.model = it->second;
        out.lobby_players.push_back(lp);
    }
}

bool Game::has_finished() const { return !gameloop.is_alive(); }

void Game::on_disconnect(int id) {
    registry.remove(id);

    if (!is_started()) {
        remove_lobby_player(id);

        // si seguimos en la lobby notificamos que se fue alguien

        LobbySnapshotData lobby_snap;
        build_lobby_snapshot(static_cast<uint32_t>(this->lobby_id), lobby_snap);

        auto snap = std::make_shared<LobbySnapshotEvent>(std::move(lobby_snap));
        registry.broadcast(snap);
        return;
    }

    // Si la partida esta en curso, le pusheamos al gameloop para que lo desconecte
    CommandReceiver cmd{};
    cmd.type = CommandReceiverType::Disconect;
    cmd.client_id = id;
    cmd.param = 0;

    try {
        command_queue.push(cmd);
    } catch (const ClosedQueue&) {
        // La partida ya esta cerrandose, ignoramos total no cambia nada
    }
}

bool Game::is_empty() { return registry.size() == 0; }

void Game::add_lobby_player(int id, uint8_t model, const std::string& name) {
    lobby_players[id] = model;
    names[id] = name;
}

void Game::remove_lobby_player(int id) { lobby_players.erase(id); }
