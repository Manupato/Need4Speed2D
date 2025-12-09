#include "config.h"

#include <iostream>

Config::Config(const std::string& path) {
    try {

        // Atributos por defecto por si falla el YAML
        race_total_time_ = 610.0f;
        race_countdown_time_ = 10.0f;
        results_screen_seconds_ = 10.0f;
        upgrades_screen_seconds_ = 10.0f;

        car_designs_ = {
                {0, {{25.0f, 45.0f, 50.0f, 40.0f, 10.0f}, 1.75f, 1.25f}},
                {1, {{30.0f, 50.0f, 70.0f, 50.0f, 15.0f}, 2.5f, 1.25f}},
                {2, {{50.0f, 70.0f, 65.0f, 50.0f, 20.0f}, 2.5f, 1.25f}},
                {3, {{70.0f, 90.0f, 70.0f, 50.0f, 25.0f}, 2.5f, 1.25f}},
                {4, {{25.0f, 40.0f, 60.0f, 100.0f, 80.0f}, 2.4375f, 1.5f}},
                {5, {{90.0f, 100.0f, 80.0f, 40.0f, 30.0f}, 2.5f, 1.375f}},
                {6, {{30.0f, 60.0f, 70.0f, 90.0f, 60.0f}, 3.0f, 1.25f}},
        };

        upgrade_penalties_ = {
                {1, 10.0},
                {2, 20.0},
                {3, 30.0},
        };

        max_npcs_moving = 10;
        max_npcs_parking = 10;
        npc_speed = 6.0f;
        npc_model = 0;
        min_distance_to_pole = 20.0f;

        physics_time_step_ = 1.0f / 60.0f;
        physics_substeps_ = 4;
        hit_event_threshold_ = 6.0f;


        slow_zone_factor_ = 0.4;
        reverse_factor_ = 0.6;

        damage_low_ = 1.0;
        damage_med_ = 3.0;
        damage_high_ = 5.0;

        min_density_ = 0.7;
        max_density_ = 1.5;
        min_max_speed_ = 90.0;
        max_max_speed_ = 220.0;
        min_engine_force_ = 6.0;
        max_engine_force_ = 40.0;
        min_turn_torque_ = 8.0;
        max_turn_torque_ = 20.0;
        min_friction_ = 0.05;
        max_friction_ = 0.2;
        min_shield_ = 0.0;
        max_shield_ = 0.8;

        root = YAML::LoadFile(path);

        load_game_config();
        load_car_designs();
        load_penalties_config();
        load_npcs_config();
        load_car_tuning();
    } catch (const std::exception& e) {
        std::cerr << "Config: error cargando config.yaml: " << e.what()
                  << " (usando valores por defecto)" << std::endl;
    } catch (...) {
        std::cerr << "Config: error desconocido cargando config.yaml (usando valores por defecto)"
                  << std::endl;
    }
}

void Config::load_game_config() {
    auto game = root["game"];
    if (!game) {
        return;
    }

    race_total_time_ = game["race_total_time"].as<float>(race_total_time_);
    race_countdown_time_ = game["race_countdown_time"].as<float>(race_countdown_time_);
    results_screen_seconds_ = game["results_screen_seconds"].as<float>(results_screen_seconds_);
    upgrades_screen_seconds_ = game["upgrades_screen_seconds"].as<float>(upgrades_screen_seconds_);

    auto physics = game["physics"];
    if (physics) {
        float ts = physics["timestep"].as<float>(physics_time_step_);
        int ss = physics["substeps"].as<int>(physics_substeps_);
        float ht = physics["hit_event_threshold"].as<float>(hit_event_threshold_);

        if (ts > 0.0f)
            physics_time_step_ = ts;
        if (ss >= 1)
            physics_substeps_ = ss;
        if (ht >= 0.0f)
            hit_event_threshold_ = ht;
    }
}

void Config::load_car_designs() {
    auto cars = root["cars"];
    if (!cars) {
        return;
    }

    for (const auto& it: cars) {
        uint16_t id = it.first.as<uint16_t>();
        const YAML::Node& node = it.second;

        CarDesignDef def = {};
        auto default_it = car_designs_.find(id);
        if (default_it != car_designs_.end()) {
            def = default_it->second;
        }

        if (node["base_length"]) {
            def.baseLength = node["base_length"].as<float>(def.baseLength);
        }
        if (node["base_width"]) {
            def.baseWidth = node["base_width"].as<float>(def.baseWidth);
        }

        auto stats = node["stats"];
        if (stats) {
            def.stats.speed = stats["speed"].as<float>(def.stats.speed);
            def.stats.engine_force = stats["engine_force"].as<float>(def.stats.engine_force);
            def.stats.handling = stats["handling"].as<float>(def.stats.handling);
            def.stats.weight = stats["weight"].as<float>(def.stats.weight);
            def.stats.shield = stats["shield"].as<float>(def.stats.shield);
        }

        car_designs_[id] = def;
    }
}

const CarDesignDef& Config::car_design(uint16_t id) const {
    auto it = car_designs_.find(id);
    if (it != car_designs_.end()) {
        return it->second;
    }

    auto fallback = car_designs_.find(0);
    if (fallback != car_designs_.end()) {
        return fallback->second;
    }
    return car_designs_.begin()->second;
}

void Config::load_penalties_config() {
    auto penalties = root["upgrades"]["penalties"];
    if (!penalties) {
        return;
    }

    for (auto it = penalties.begin(); it != penalties.end(); ++it) {
        uint8_t code = static_cast<uint8_t>(it->first.as<int>());
        double seconds = it->second.as<double>();
        upgrade_penalties_[code] = seconds;
    }
}

double Config::upgrade_penalty_for(uint8_t code) const {
    auto it = upgrade_penalties_.find(code);
    if (it == upgrade_penalties_.end()) {
        return 0.0;
    }
    return it->second;
}

void Config::load_npcs_config() {
    auto npcs = root["npcs"];
    if (!npcs) {
        return;
    }

    max_npcs_moving = npcs["max_npcs_moving"].as<int>(max_npcs_moving);
    max_npcs_parking = npcs["max_npcs_parking"].as<int>(max_npcs_parking);
    npc_speed = npcs["npc_speed"].as<float>(npc_speed);
    npc_model = npcs["npc_model"].as<int>(npc_model);
    min_distance_to_pole = npcs["min_distance_to_pole"].as<float>(min_distance_to_pole);
}

void Config::load_car_tuning() {
    auto car_tuning = root["car_tuning"];
    if (!car_tuning) {
        return;
    }

    slow_zone_factor_ = car_tuning["slow_zone_factor"].as<float>(slow_zone_factor_);
    reverse_factor_ = car_tuning["reverse_factor"].as<float>(reverse_factor_);

    auto crash = car_tuning["crash"];
    if (crash) {
        damage_low_ = crash["damage_low"].as<float>(damage_low_);
        damage_med_ = crash["damage_med"].as<float>(damage_med_);
        damage_high_ = crash["damage_high"].as<float>(damage_high_);
    }

    auto mapping = car_tuning["mapping"];
    if (mapping) {
        min_density_ = mapping["min_density"].as<float>(min_density_);
        max_density_ = mapping["max_density"].as<float>(max_density_);
        min_max_speed_ = mapping["min_max_speed"].as<float>(min_max_speed_);
        max_max_speed_ = mapping["max_max_speed"].as<float>(max_max_speed_);
        min_engine_force_ = mapping["min_engine_force"].as<float>(min_engine_force_);
        max_engine_force_ = mapping["max_engine_force"].as<float>(max_engine_force_);
        min_turn_torque_ = mapping["min_turn_torque"].as<float>(min_turn_torque_);
        max_turn_torque_ = mapping["max_turn_torque"].as<float>(max_turn_torque_);
        min_friction_ = mapping["min_friction"].as<float>(min_friction_);
        max_friction_ = mapping["max_friction"].as<float>(max_friction_);
        min_shield_ = mapping["min_shield"].as<float>(min_shield_);
        max_shield_ = mapping["max_shield"].as<float>(max_shield_);
    }
}
