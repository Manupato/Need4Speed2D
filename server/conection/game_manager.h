#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "game.h"

// Unico manegador de partidas de todo el server.
class GameManager {
private:
    // Recurso compartido, necesita mutex.
    std::mutex m;
    std::map<int, std::unique_ptr<Game>> games;
    const int default_id = 1001;

public:
    GameManager() = default;

    // Crea una nueva lobby y mete al jugador.
    // Devuelve true si fue exitosa.
    bool create_lobby_and_join(int client_id, uint8_t model, Queue<std::shared_ptr<IEvent>>& out_q,
                               Game*& game, LobbySnapshotData& snapshot,
                               std::vector<std::string>& maps, const std::string& name);

    // Une al jugador a una lobby existente.
    bool join_lobby(int client_id, uint32_t lobby_id, uint8_t model,
                    Queue<std::shared_ptr<IEvent>>& out_q, Game*& game, LobbySnapshotData& snapshot,
                    const std::string& name);

    void reap_finished_games();

    // Intenta iniciar la lobby
    bool start_lobby(uint32_t lobby_id);

    void disconnect(const int id_);

    // Apagado ordenado de todas las partidas
    void stop_all();
    void join_all();
};

#endif  // GAME_MANAGER_H
