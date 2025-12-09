#include "snapshot_builder.h"

#include <algorithm>
#include <list>

SnapshotBuilder::SnapshotBuilder(ClientRegistryMonitor& registry, PhysicWorld& physics):
        physics(physics), registry(registry) {}

void SnapshotBuilder::send_snapshot(double& snapshot_acumulate, const float snapshot_interval,
                                    const std::map<int, Car>& cars,
                                    std::map<int, RaceProgress>& race_progress,
                                    double race_with_countdown, const std::list<Car>& npc_cars) {

    if (cars.empty()) {
        snapshot_acumulate -= snapshot_interval;
        return;
    }

    GameSnapshotData data;

    if (race_with_countdown <= 0) {
        data.time_seconds_remained = 0;
    } else {
        data.time_seconds_remained = static_cast<uint32_t>(race_with_countdown);
    }

    data.players.reserve(cars.size());

    for (const auto& [player_id, car]: cars) {
        add_car_to_snapshot(car, player_id, race_progress, data);
    }

    for (const Car& npc: npc_cars) {
        add_npc_to_snapshot(npc, data);
    }


    if (!data.players.empty()) {
        auto ev = std::make_shared<GameSnapshotEvent>(std::move(data));
        registry.broadcast(ev);
    }

    snapshot_acumulate -= snapshot_interval;
}

void SnapshotBuilder::add_npc_to_snapshot(const Car& car, GameSnapshotData& snapshot) {
    b2Vec2 pos = car.get_position();
    b2Rot rot = car.get_rotation();

    uint32_t x_px = static_cast<uint32_t>(pos.x * PPM);
    uint32_t y_px = static_cast<uint32_t>((physics.getHeightInMeters() - pos.y) * PPM);

    float angle_rad = b2Rot_GetAngle(rot);
    float angle_deg = angle_rad * (180.0f / 3.14159265f);

    NpcSnapshot ps;
    ps.model = car.get_model();
    ps.animation = (car.is_destroyed()) ? static_cast<uint8_t>(car.get_one_destroy()) :
                                          static_cast<uint8_t>(car.get_and_consume_actual_crash());

    ps.x_px = x_px;
    ps.y_px = y_px;
    ps.z = static_cast<uint8_t>(car.get_level());
    ps.angle = static_cast<uint32_t>(angle_between_0_and_360(angle_deg));

    snapshot.npcs.push_back(ps);
}

void SnapshotBuilder::add_car_to_snapshot(const Car& car, const int& player_id,
                                          std::map<int, RaceProgress>& race_progress,
                                          GameSnapshotData& snapshot) {
    b2Vec2 pos = car.get_position();
    b2Rot rot = car.get_rotation();

    uint32_t x_px = static_cast<uint32_t>(pos.x * PPM);  // PPM = 16 px/m
    uint32_t y_px = static_cast<uint32_t>((physics.getHeightInMeters() - pos.y) * PPM);

    float angle_rad = b2Rot_GetAngle(rot);
    float angle_deg = angle_rad * (180.0f / 3.14159265f);

    PlayerSnapshot ps;
    ps.id = static_cast<uint32_t>(player_id);
    if (car.is_ghost()) {
        ps.ghost = 1;
    }
    ps.car_life = (car.is_destroyed()) ? 0 : (static_cast<uint16_t>(car.get_health()));
    ps.model = car.get_model();
    ps.animation = (car.is_destroyed()) ? static_cast<uint8_t>(car.get_one_destroy()) :
                                          static_cast<uint8_t>(car.get_and_consume_actual_crash());

    uint8_t sound = 0;
    if (car.consume_goal_sound()) {
        sound = 2;  // meta tiene prioridad
    } else if (car.consume_brake_sound()) {
        sound = 1;  // frenada
    }
    ps.sound_code = sound;

    ps.x_px = x_px;
    ps.y_px = y_px;
    ps.z = static_cast<uint8_t>(car.get_level());
    ps.angle = static_cast<uint32_t>(angle_between_0_and_360(angle_deg));

    // Iteramos todos los sensores de checkpoint del mapa
    const auto& sens = physics.get_sensors();

    int max_order = 0;
    if (!sens.empty()) {
        auto it = std::max_element(sens.begin(), sens.end(), [](const auto& a, const auto& b) {
            return *a.get_orderPtr() < *b.get_orderPtr();
        });
        max_order = *it->get_orderPtr();
    }

    RaceProgress rp = race_progress[player_id];
    int next_checkpoint = rp.next_order;

    int next_next_checkpoint = -1;
    ps.there_is_second_checkpoint = 0;

    if (next_checkpoint < max_order) {
        next_next_checkpoint = next_checkpoint + 1;
    }

    for (const auto& s: sens) {
        if (next_checkpoint == *s.get_orderPtr()) {

            ps.next_checkpoint.push_back(
                    Coord{.x_px = static_cast<uint32_t>(s.get_position_in_px_X()),
                          .y_px = static_cast<uint32_t>(s.get_position_in_px_Y())});
            if (s.is_goal()) {
                ps.goal = 0x01;
            }
        } else if (next_next_checkpoint != -1 && next_next_checkpoint == *s.get_orderPtr()) {
            ps.there_is_second_checkpoint = 1;
            ps.next_next_checkpoint.push_back(
                    Coord{.x_px = static_cast<uint32_t>(s.get_position_in_px_X()),
                          .y_px = static_cast<uint32_t>(s.get_position_in_px_Y())});
            if (s.is_goal()) {
                ps.next_next_goal = 0x01;
            }
        }
    }
    snapshot.players.push_back(ps);
}

float SnapshotBuilder::angle_between_0_and_360(float angle) {
    while (angle < 0) angle += 360.0f;
    while (angle >= 360.0f) angle -= 360.0f;

    return angle;
}

void SnapshotBuilder::send_pre_game_snapshot(const int remaining, const double race_total_time,
                                             const double race_duration, MapId map_id,
                                             const PoleCoordsAndDirec& pole_position) {
    PreGameSnapshotData data;
    data.remaining_races = static_cast<uint16_t>(remaining);
    data.map_id = map_id;
    data.pole = pole_position;
    data.race_total_time_seconds = static_cast<uint32_t>(race_total_time);
    data.race_move_enabled_time_seconds = static_cast<uint32_t>(race_duration);

    auto ev = std::make_shared<PreGameSnapshotEvent>(std::move(data));
    registry.broadcast(ev);
}

void SnapshotBuilder::send_race_results(const std::vector<PlayerRaceResult>& results,
                                        bool last_race) {
    RaceResultsData data;
    data.last_race = last_race ? 1 : 0;
    data.race_results = results;
    data.podium_count = 0;

    auto ev = std::make_shared<RaceResultsEvent>(std::move(data));
    registry.broadcast(ev);
}

void SnapshotBuilder::send_race_results(const std::vector<PlayerRaceResult>& results,
                                        bool last_race, int result_snapshot_sent) {

    const int n = static_cast<int>(results.size());
    if (n == 0)
        return;

    RaceResultsData data;

    uint8_t podium_count = static_cast<uint8_t>(
            std::min(result_snapshot_sent, static_cast<int>(MAX_PODIUM_SLOTS)));
    data.last_race = last_race ? 1 : 0;
    data.race_results = results;
    data.podium_count = podium_count;

    auto ev = std::make_shared<RaceResultsLastEvent>(std::move(data));
    registry.broadcast(ev);
}
