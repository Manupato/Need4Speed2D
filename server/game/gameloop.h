#ifndef GAMELOOP_H
#define GAMELOOP_H

#include <chrono>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include <stdio.h>

#include "../../common/queue.h"
#include "../../common/thread.h"
#include "../command.h"
#include "../conection/client_registry.h"
#include "../server_error.h"

#include "car.h"
#include "physic_world.h"
#include "race_context.h"
#include "race_progress.h"
#include "race_system.h"
#include "snapshot_builder.h"
#include "world_state.h"

enum class RaceState {
    WaitingForLobbyStart,
    Running,
    ShowingResults,
    ChoosingUpgrades,
    WaitingUpgrades,
    ShowingResultsLastRace,
    Finished
};

// Toda la informacion del jugador en la PARTIDA
struct PlayerSession {
    std::string name;
    std::vector<uint8_t> applied_upgrades;  // todas las upgrades que se le fueron aplicando a lo
                                            // largo de la PARTIDA
    bool upgrade_received_this_round = false;
    uint8_t pending_upgrade = 0;
    bool disconnected = false;
    double total_time = 0.0;
    int races_finished = 0;
};

class Gameloop: public Thread {
private:
    Queue<CommandReceiver>& command_queue;
    ClientRegistryMonitor& registry;
    std::vector<std::string> maps;
    std::size_t current_map_index{0};

    // Contexto de la carrera actual
    std::unique_ptr<RaceContext> race;

    // Estado inicial del gameloop.
    RaceState state{RaceState::WaitingForLobbyStart};

    // Tiempos que ira administrando el gameloop
    double race_with_countdown = 0.0;
    double results_time_remaining = 0.0;

    // Tiempos para mostrar la tabla final de a poco
    // Primero del 8 al 4. Despues al 3, 2 y 1 (Son 4 snapshots distintas)
    double time_each_result_snapshot = 0.0;
    double actual_result_time = 0.0;
    int steps_result_table = 4;
    int result_snapshot_sent = 0;
    std::vector<PlayerRaceResult> last_results;

    // Procesa todos los comandos disponibles en la cola
    void receive_commands();
    void receive_command_move(const CommandReceiver& cmd);
    void upgrade_car(const CommandReceiver& cmd);
    void receive_new_car(const CommandReceiver& cmd);
    void disconect_car(const CommandReceiver& cmd);
    void begin_race();

    void race_finished();
    void update_state_running(double dt);
    void update_state_showing_results(double dt);
    void update_state_choosing_upgrades(double dt);
    void update_state_waiting_upgrades(double dt);
    void start_new_map();
    void update_state_showing_resultes_last_race(double dt);
    void send_pre_game_snapshot();


    void add_all_results(const std::map<int, Car>& cars, std::map<int, RaceProgress>& rp_map,
                         std::vector<PlayerRaceResult>& results);

    void create_new_race(const std::map<int, Car>& cars);

    float race_total_time;
    float race_countdown_time;
    float results_screen_seconds;
    float upgrades_screen_seconds;

    // Mapea id del cliente con su informacion en la partida
    std::map<int, PlayerSession> players;

    void update_state(double dt);
    void step_simulation(double& acumulate, double delta_time);
    void send_snapshots(double& snapshot_acumulate, float snapshot_interval);

public:
    Gameloop(Queue<CommandReceiver>& command_queue, ClientRegistryMonitor& registry,
             std::vector<std::string> maps);

    void run() override;

    ~Gameloop() override = default;
    Gameloop(const Gameloop&) = delete;
    Gameloop& operator=(const Gameloop&) = delete;
};

#endif  // GAMELOOP_H
