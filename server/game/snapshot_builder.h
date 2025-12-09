#ifndef SNAPSHOT_BUILDER_H
#define SNAPSHOT_BUILDER_H

#include <list>
#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "../conection/client_registry.h"
#include "../event.h"

#include "car.h"
#include "physic_world.h"
#include "race_progress.h"

class SnapshotBuilder {

private:
    PhysicWorld& physics;

    ClientRegistryMonitor& registry;

    // Maximo siempre 3 slots de podio
    static constexpr uint8_t MAX_PODIUM_SLOTS = 3;

    // Convierte el angulo a un indice de rotacion
    float angle_between_0_and_360(float angle);

    void add_car_to_snapshot(const Car& car, const int& player_id,
                             std::map<int, RaceProgress>& race_progress,
                             GameSnapshotData& snapshot);

    void add_npc_to_snapshot(const Car& car, GameSnapshotData& snapshot);

    static constexpr float PPM = 16.0f;


public:
    SnapshotBuilder(ClientRegistryMonitor& registry, PhysicWorld& physics);

    void send_snapshot(double& snapshot_acumulate, const float snapshot_interval,
                       const std::map<int, Car>& cars, std::map<int, RaceProgress>& race_progress,
                       double race_with_countdown, const std::list<Car>& npc_cars);

    void send_pre_game_snapshot(const int remaining, const double race_total_time,
                                const double race_duration, MapId map_id,
                                const PoleCoordsAndDirec& pole_position);

    void send_race_results(const std::vector<PlayerRaceResult>& results, bool last_race);

    void send_race_results(const std::vector<PlayerRaceResult>& results, bool last_race,
                           int result_snapshot_sent);

    ~SnapshotBuilder() = default;
    SnapshotBuilder(const SnapshotBuilder& other) = delete;
    SnapshotBuilder& operator=(const SnapshotBuilder& other) = delete;
};

#endif  // SNAPSHOT_BUILDER_H
