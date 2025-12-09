#ifndef RACE_CONTEXT_H
#define RACE_CONTEXT_H

#include <cmath>
#include <map>
#include <string>
#include <vector>

#include "../command.h"
#include "../conection/client_registry.h"
#include "../server_error.h"

#include "car.h"
#include "physic_world.h"
#include "race_progress.h"
#include "race_system.h"
#include "snapshot_builder.h"
#include "world_state.h"

// Mientras que el gameloop "orquesta" la partida en general, el RaceContext es la "fachada"
// de la carrera, exponiendo metodos de alto nivel que usara el gameloop.
class RaceContext {
private:
    std::string map_path;

    PhysicWorld physics;
    WorldState world_state;
    SnapshotBuilder snapshot_builder;
    RaceSystem race_system;

    void init_npcs();

public:
    RaceContext(const std::string& map_path, ClientRegistryMonitor& registry);

    // Un step de fisica
    void step_physics();

    // Aplica inputs de los jugadores al world_state
    void apply_player_inputs();

    // Maneja checkpoints, colisiones, puentes
    void handle_race_and_contacts(double race_with_countdown_actual);

    // Manda snapshot al cliente
    void send_snapshot(double& snapshot_acumulate, float snapshot_interval,
                       double race_with_countdown);

    void send_pre_game_snapshot(const int remaining, const double race_total_time,
                                const double race_duration);

    void send_race_results(const std::vector<PlayerRaceResult>& results, bool last_race);

    void send_race_results(const std::vector<PlayerRaceResult>& results, bool last_race,
                           int result_snapshot_sent);

    // Crear un auto nuevo para un jugador
    void spawn_car_for_player(int player_id, uint16_t model);

    std::map<int, Car>& get_cars();
    std::map<int, RaceProgress>& get_race_progress();
    const PhysicWorld& get_physics() const { return physics; }

    void set_race_finish(int id_player, double time_finish);

    void receive_command_move(const CommandReceiver& cmd, double race_with_countdown);

    bool all_players_finished_or_dead() const;

    MapId get_map_id();

    PoleCoordsAndDirec get_pole_position();

    void upgrade_car(int player_id, uint8_t upgrade);

    void kill(int player_id);

    void update_npcs();

    static NpcDir dir_from_angle(float angle);

    float get_time_step() const { return physics.getTimeStep(); }
};

#endif
