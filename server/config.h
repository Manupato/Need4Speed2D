#ifndef CONFIG_H
#define CONFIG_H

#include <cstdint>
#include <map>
#include <string>

#include <yaml-cpp/yaml.h>

#include "../common/resource_paths.h"
#include "game/car_design.h"

class Config {
private:
    explicit Config(const std::string& path);

    YAML::Node root;

    float race_total_time_;
    float race_countdown_time_;
    float results_screen_seconds_;
    float upgrades_screen_seconds_;

    std::map<uint16_t, CarDesignDef> car_designs_;
    void load_game_config();
    void load_car_designs();
    void load_penalties_config();
    void load_npcs_config();
    void load_car_tuning();

    std::map<uint8_t, double> upgrade_penalties_;

    int max_npcs_moving;
    int max_npcs_parking;
    float npc_speed;
    int npc_model;
    float min_distance_to_pole;

    float physics_time_step_;
    int physics_substeps_;
    float hit_event_threshold_;

    float slow_zone_factor_;
    float reverse_factor_;

    float damage_low_;
    float damage_med_;
    float damage_high_;

    float min_density_;
    float max_density_;
    float min_max_speed_;
    float max_max_speed_;
    float min_engine_force_;
    float max_engine_force_;
    float min_turn_torque_;
    float max_turn_torque_;
    float min_friction_;
    float max_friction_;
    float min_shield_;
    float max_shield_;

public:
    static Config& instance() {
        static Config cfg{ResourcePaths::config() + "/config.yaml"};
        return cfg;
    }

    float race_total_time() const { return race_total_time_; }
    float race_countdown_time() const { return race_countdown_time_; }
    float results_screen_seconds() const { return results_screen_seconds_; }
    float upgrades_screen_seconds() const { return upgrades_screen_seconds_; }
    const std::map<uint16_t, CarDesignDef>& car_designs() const { return car_designs_; }
    const CarDesignDef& car_design(uint16_t id) const;
    double upgrade_penalty_for(uint8_t code) const;

    int get_max_npcs_moving() const { return max_npcs_moving; }
    int get_max_npcs_parking() const { return max_npcs_parking; }
    float get_npc_speed() const { return npc_speed; }
    int get_npc_model() const { return npc_model; }
    float get_min_distance_to_pole() const { return min_distance_to_pole; }

    float physics_time_step() const { return physics_time_step_; }
    int physics_substeps() const { return physics_substeps_; }
    float hit_event_threshold() const { return hit_event_threshold_; }

    float slow_zone_factor() const { return slow_zone_factor_; }
    float reverse_factor() const { return reverse_factor_; }

    float damage_low() const { return damage_low_; }
    float damage_med() const { return damage_med_; }
    float damage_high() const { return damage_high_; }

    float min_density() const { return min_density_; }
    float max_density() const { return max_density_; }
    float min_max_speed() const { return min_max_speed_; }
    float max_max_speed() const { return max_max_speed_; }
    float min_engine_force() const { return min_engine_force_; }
    float max_engine_force() const { return max_engine_force_; }
    float min_turn_torque() const { return min_turn_torque_; }
    float max_turn_torque() const { return max_turn_torque_; }
    float min_friction() const { return min_friction_; }
    float max_friction() const { return max_friction_; }
    float min_shield() const { return min_shield_; }
    float max_shield() const { return max_shield_; }
};

#endif  // CONFIG_H
