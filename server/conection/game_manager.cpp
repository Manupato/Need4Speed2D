#include "game_manager.h"

#include <string>
#include <utility>

static void broadcast_lobby_snapshot(Game& game, uint32_t lobby_id) {
    LobbySnapshotData data;
    game.build_lobby_snapshot(lobby_id, data);
    auto ev = std::make_shared<LobbySnapshotEvent>(std::move(data));
    ClientRegistryMonitor& reg = game.get_registry();
    reg.broadcast(ev);
}

static int next_lobby_id(const std::map<int, std::unique_ptr<Game>>& games, int base) {
    int id = base;
    while (games.find(id) != games.end()) {
        ++id;
    }
    return id;
}

bool GameManager::start_lobby(uint32_t lobby_id) {
    std::lock_guard<std::mutex> lk(m);

    auto it = games.find((int)lobby_id);
    if (it == games.end()) {
        return false;
    }
    Game* ptr = it->second.get();

    if (!ptr->start_lobby())
        return false;

    // Aviso a todos los que estaban en esa lobby
    auto ev = std::make_shared<StartLobbyEvent>();
    ClientRegistryMonitor& reg = ptr->get_registry();
    reg.broadcast(ev);
    return true;
}

void GameManager::stop_all() {
    std::vector<Game*> to_stop;
    {
        std::lock_guard<std::mutex> lk(m);
        to_stop.reserve(games.size());
        for (auto& kv: games)
            if (kv.second)
                to_stop.push_back(kv.second.get());
    }
    for (Game* g: to_stop) g->stop();
}

void GameManager::join_all() {
    std::vector<Game*> to_join;
    {
        std::lock_guard<std::mutex> lk(m);
        to_join.reserve(games.size());
        for (auto& kv: games)
            if (kv.second)
                to_join.push_back(kv.second.get());
    }
    for (Game* g: to_join) g->join();
}

bool GameManager::create_lobby_and_join(int client_id, uint8_t model,
                                        Queue<std::shared_ptr<IEvent>>& out_q, Game*& game,
                                        LobbySnapshotData& snapshot, std::vector<std::string>& maps,
                                        const std::string& name) {
    std::lock_guard<std::mutex> lk(m);
    const int lobby_id = next_lobby_id(games, default_id);

    std::unique_ptr<Game> g(new Game(maps, lobby_id));
    g->start();
    Game* ptr = g.get();
    games.emplace(lobby_id, std::move(g));

    ptr->get_registry().add(client_id, out_q);
    ptr->add_lobby_player(client_id, model, name);

    auto ev = std::make_shared<ExitJoinEvent>();
    out_q.push(ev);

    ptr->build_lobby_snapshot((uint32_t)lobby_id, snapshot);

    broadcast_lobby_snapshot(*ptr, (uint32_t)lobby_id);

    game = ptr;
    return true;
}

bool GameManager::join_lobby(int client_id, uint32_t lobby_id, uint8_t model,
                             Queue<std::shared_ptr<IEvent>>& out_q, Game*& game,
                             LobbySnapshotData& snapshot, const std::string& name) {
    std::lock_guard<std::mutex> lk(m);
    auto it = games.find((int)lobby_id);
    if (it == games.end())
        return false;

    Game* ptr = it->second.get();
    if (!ptr->can_join())
        return false;
    if (ptr->has_player(client_id))
        return false;

    // Registrar salida y agregar a lobby
    ptr->get_registry().add(client_id, out_q);
    ptr->add_lobby_player(client_id, model, name);

    auto ev = std::make_shared<ExitJoinEvent>();
    out_q.push(ev);

    ptr->build_lobby_snapshot(lobby_id, snapshot);
    broadcast_lobby_snapshot(*ptr, lobby_id);
    game = ptr;
    return true;
}

void GameManager::reap_finished_games() {
    std::vector<std::unique_ptr<Game>> finished;
    {
        std::lock_guard<std::mutex> lk(m);
        for (auto it = games.begin(); it != games.end();) {
            Game* g = it->second.get();
            if (g && g->has_finished() && g->is_empty()) {
                finished.push_back(std::move(it->second));
                it = games.erase(it);
            } else {
                ++it;
            }
        }
    }
    for (auto& g: finished) {
        g->join();
    }
}

void GameManager::disconnect(const int id_) {
    std::unique_ptr<Game> to_join;

    {
        std::lock_guard<std::mutex> lk(m);

        for (auto it = games.begin(); it != games.end(); ++it) {
            Game* g = it->second.get();
            if (!g)
                continue;

            if (g->has_player(id_)) {
                g->on_disconnect(id_);

                // Si quedo vacia, la la borramos
                if (g->is_empty()) {
                    to_join = std::move(it->second);
                    games.erase(it);
                }
                break;
            }
        }
    }

    // Fuera del lock, hacemos join si se quedo sin nadie la partida
    if (to_join) {
        to_join->stop();
        to_join->join();
    }
}
