#include "race_context.h"

#include <algorithm>
#include <utility>

#include "../../common/resource_paths.h"
#include "../config.h"

RaceContext::RaceContext(const std::string& map_path, ClientRegistryMonitor& registry):
        map_path(map_path),
        physics(ResourcePaths::userMaps() + "/" + map_path),
        world_state(physics),
        snapshot_builder(registry, physics),
        race_system(physics, world_state) {

    physics.init_world();
    init_npcs();
}

void RaceContext::init_npcs() {
    auto npc_spawns =
            physics.get_npc_spawns_filtered(Config::instance().get_min_distance_to_pole());
    auto npc_park_spawns =
            physics.get_npc_park_spawns_filtered(Config::instance().get_min_distance_to_pole());

    int mnpcm = std::max(0, Config::instance().get_max_npcs_moving());
    std::size_t max_npcs_moving = static_cast<std::size_t>(mnpcm);
    std::size_t count = static_cast<std::size_t>(std::min(max_npcs_moving, npc_spawns.size()));
    int mnpcp = std::max(0, Config::instance().get_max_npcs_parking());
    std::size_t max_npcs_parking = static_cast<std::size_t>(mnpcp);
    std::size_t count_parked =
            static_cast<std::size_t>(std::min(max_npcs_parking, npc_park_spawns.size()));

    float npc_speed = Config::instance().get_npc_speed();
    uint16_t npc_model = static_cast<uint16_t>(Config::instance().get_npc_model());

    for (std::size_t i = 0; i < count; ++i) {
        Spawn s = npc_spawns[i];
        NpcDir dir = dir_from_angle(s.angle_rad);
        world_state.spawn_npc(std::move(s), npc_model, dir, npc_speed);
    }
    for (std::size_t i = 0; i < count_parked; ++i) {
        Spawn s = npc_park_spawns[i];
        NpcDir dir = dir_from_angle(s.angle_rad);
        world_state.spawn_npc(std::move(s), npc_model, dir, 0.0);
    }
}

NpcDir RaceContext::dir_from_angle(float angle) {
    constexpr float PI = 3.14159265358979323846;
    constexpr float HALF_PI = PI / 2;

    if (angle >= -HALF_PI && angle < HALF_PI) {
        return NpcDir::Right;
    } else if (angle >= HALF_PI && angle < PI) {
        return NpcDir::Up;
    } else if (angle >= -PI && angle < -HALF_PI) {
        return NpcDir::Down;
    } else {
        return NpcDir::Left;
    }
}

void RaceContext::apply_player_inputs() { world_state.apply_player_inputs(); }

void RaceContext::step_physics() { physics.step(); }

void RaceContext::handle_race_and_contacts(double race_with_countdown_actual) {
    race_system.handle_checkpoint_contacts(race_with_countdown_actual);
    physics.handle_contacts();
}

void RaceContext::send_snapshot(double& snapshot_acumulate, float snapshot_interval,
                                double race_with_countdown) {
    snapshot_builder.send_snapshot(snapshot_acumulate, snapshot_interval, world_state.get_cars(),
                                   world_state.get_race_progress(), race_with_countdown,
                                   world_state.get_npc_cars());
}

void RaceContext::send_pre_game_snapshot(const int remaining, const double race_total_time,
                                         const double race_duration) {
    snapshot_builder.send_pre_game_snapshot(remaining, race_total_time, race_duration,
                                            physics.get_map_id(), physics.get_pole_position());
}

void RaceContext::send_race_results(const std::vector<PlayerRaceResult>& results, bool last_race) {
    snapshot_builder.send_race_results(results, last_race);
}

void RaceContext::send_race_results(const std::vector<PlayerRaceResult>& results, bool last_race,
                                    int result_snapshot_sent) {
    snapshot_builder.send_race_results(results, last_race, result_snapshot_sent);
}

void RaceContext::spawn_car_for_player(int player_id, uint16_t model) {
    std::size_t spawnIndex = world_state.number_of_players();
    Spawn spawnPos = physics.get_spawn_for_index(spawnIndex);

    world_state.add_new_car(std::move(spawnPos), model, player_id, physics.getWorld());
}

std::map<int, Car>& RaceContext::get_cars() { return world_state.get_cars(); }

std::map<int, RaceProgress>& RaceContext::get_race_progress() {
    return world_state.get_race_progress();
}

void RaceContext::receive_command_move(const CommandReceiver& cmd, double race_with_countdown) {

    if (!world_state.client_have_car(cmd.client_id)) {
        return;
    }

    switch (cmd.param) {

        case 0x00:
            world_state.change_w(true, cmd.client_id);
            break;  // W
        case 0x01:
            world_state.change_a(true, cmd.client_id);
            break;  // A
        case 0x02:
            world_state.change_s(true, cmd.client_id);
            break;  // S
        case 0x03:
            world_state.change_d(true, cmd.client_id);
            break;  // D
        case 0x04:
            world_state.change_w(false, cmd.client_id);
            break;  // ~W
        case 0x05:
            world_state.change_a(false, cmd.client_id);
            break;  // ~A
        case 0x06:
            world_state.change_s(false, cmd.client_id);
            break;  // ~S
        case 0x07:
            world_state.change_d(false, cmd.client_id);
            break;  // ~D
        case 0x08:
            world_state.win(cmd.client_id, race_with_countdown);
            break;
        case 0x09:
            world_state.lose(cmd.client_id);
            break;
        case 0x10:
            world_state.undestroyable(cmd.client_id);
            break;
        case 0x11:
            world_state.ghost(cmd.client_id);
            break;

        default:
            throw ServerError("RaceContext::receive_command_move: Invalid direction code");
    }
}

MapId RaceContext::get_map_id() { return physics.get_map_id(); }

void RaceContext::set_race_finish(int id_player, double time_finish) {
    world_state.set_race_finish(id_player, time_finish);
}

bool RaceContext::all_players_finished_or_dead() const {
    return race_system.all_players_finished_or_dead();
}

PoleCoordsAndDirec RaceContext::get_pole_position() { return physics.get_pole_position(); }

void RaceContext::upgrade_car(int player_id, uint8_t upgrade) {
    auto& cars = world_state.get_cars();
    auto it = cars.find(player_id);
    if (it == cars.end())
        return;
    it->second.apply_upgrade(upgrade);
}

void RaceContext::kill(int player_id) { world_state.lose(player_id); }

void RaceContext::update_npcs() { world_state.update_npcs(); }
