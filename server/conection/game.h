#ifndef GAME_H
#define GAME_H

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "../../common/queue.h"
#include "../command.h"
#include "../event.h"
#include "../game/gameloop.h"

#include "client_registry.h"

// Cada partida tiene su propio gameloop, cola de comandos y registry.
// El game manager tiene varias partidas (lobbies) activas, creandolas y destruyendolas segun
// sea necesario. Permite ademas, a los clientes unirse a ellas antes de que empiecen.

class Game {
private:
    // Recursos compartidos por los hilos
    // Cola de comandos para el gameloop
    Queue<CommandReceiver> command_queue;

    // Mapea id de cliente a su cola de eventos de salida
    ClientRegistryMonitor registry;

    // Unico hilo del juego
    Gameloop gameloop;

    // id -> modelo
    std::map<int, uint8_t> lobby_players;
    // id -> nombre
    std::map<int, std::string> names;

    int lobby_id;
    bool lobby_started{false};
    static const int size_max_players = 8;

public:
    Game(std::vector<std::string>& maps, int lobby_id);

    // ciclo de vida de una partida
    void start();
    void stop();
    void join();

    Queue<CommandReceiver>& get_cmd_q();
    ClientRegistryMonitor& get_registry();
    const Queue<CommandReceiver>& get_cmd_q() const;
    const ClientRegistryMonitor& get_registry() const;

    bool can_join() const { return !lobby_started && (int)lobby_players.size() < size_max_players; }
    bool has_player(int id) const { return lobby_players.find(id) != lobby_players.end(); }
    void add_lobby_player(int id, uint8_t model, const std::string& name);
    void remove_lobby_player(int id);
    bool is_started() const { return lobby_started; }

    void build_lobby_snapshot(uint32_t lobby_id, LobbySnapshotData& out) const;
    bool has_finished() const;
    // setea lobby_started=true, retorna false si ya estaba empezada
    bool start_lobby();

    bool is_empty();
    void on_disconnect(int id);
};

#endif  // GAME_H
