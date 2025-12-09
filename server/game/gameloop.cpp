#include "gameloop.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

#include "../config.h"

Gameloop::Gameloop(Queue<CommandReceiver>& command_queue, ClientRegistryMonitor& registry,
                   std::vector<std::string> maps):
        command_queue(command_queue),
        registry(registry),
        maps(std::move(maps)),
        race(std::make_unique<RaceContext>(this->maps.front(), registry)),
        race_total_time(Config::instance().race_total_time()),
        race_countdown_time(Config::instance().race_countdown_time()),
        results_screen_seconds(Config::instance().results_screen_seconds()),
        upgrades_screen_seconds(Config::instance().upgrades_screen_seconds()) {
    race_with_countdown = race_total_time;
    results_time_remaining = results_screen_seconds;
    time_each_result_snapshot = results_screen_seconds / 4;
}

void Gameloop::receive_commands() {
    CommandReceiver cmd;
    while (command_queue.try_pop(cmd)) {
        if (cmd.type == CommandReceiverType::Move) {
            receive_command_move(cmd);
        } else if (cmd.type == CommandReceiverType::NewCar) {
            receive_new_car(cmd);
        } else if (cmd.type == CommandReceiverType::BeginRace) {
            begin_race();
        } else if (cmd.type == CommandReceiverType::Upgrade) {
            upgrade_car(cmd);
        } else if (cmd.type == CommandReceiverType::Disconect) {
            disconect_car(cmd);
        } else {
            throw ServerError(
                    "Gameloop::receive_commands: Unknown command type received in gameloop");
        }
    }
}

void Gameloop::receive_command_move(const CommandReceiver& cmd) {
    if (state == RaceState::Running) {
        race->receive_command_move(cmd, race_with_countdown);
    }
}

void Gameloop::receive_new_car(const CommandReceiver& cmd) {
    players[cmd.client_id].name = cmd.name;
    race->spawn_car_for_player(cmd.client_id, static_cast<uint16_t>(cmd.param));
}

void Gameloop::begin_race() {
    if (state == RaceState::WaitingForLobbyStart) {
        send_pre_game_snapshot();
        state = RaceState::Running;
    }
}

void Gameloop::upgrade_car(const CommandReceiver& cmd) {
    if (state != RaceState::WaitingUpgrades) {
        return;
    }
    int client_id = cmd.client_id;
    uint8_t upgrade = cmd.param;

    players[client_id].pending_upgrade = upgrade;
    players[client_id].upgrade_received_this_round = true;
}

void Gameloop::disconect_car(const CommandReceiver& cmd) {
    players[cmd.client_id].disconnected = true;
    race->kill(cmd.client_id);
}

void Gameloop::send_pre_game_snapshot() {

    int remaining = 0;
    if (current_map_index < maps.size()) {
        remaining = (maps.size() - current_map_index);
    }
    const double race_duration = race_total_time - race_countdown_time;
    race->send_pre_game_snapshot(remaining, race_total_time, race_duration);
}

void Gameloop::add_all_results(const std::map<int, Car>& cars, std::map<int, RaceProgress>& rp_map,
                               std::vector<PlayerRaceResult>& results) {
    for (const auto& [player_id, car]: cars) {
        uint8_t status = 1;

        double race_time = race_total_time - race_countdown_time;
        auto it = rp_map.find(player_id);
        if (it != rp_map.end() && it->second.time_remaining_when_finished > 0.0) {
            double remaining = it->second.time_remaining_when_finished;
            race_time = race_time - remaining;
            if (race_time < 0)
                race_time = 0;
            status = 0;
        }

        players[player_id].total_time += race_time;
        players[player_id].races_finished += 1;

        PlayerRaceResult r;
        r.id = static_cast<uint32_t>(player_id);
        r.name = players[player_id].name;
        r.race_time_seconds = static_cast<uint32_t>(race_time);
        r.total_time_seconds = static_cast<uint32_t>(players[player_id].total_time);
        r.status = status;

        results.push_back(r);
    }
}

void Gameloop::create_new_race(const std::map<int, Car>& cars) {

    // me guardo los modelos por jugador
    std::map<int, uint16_t> player_models;
    for (const auto& [client_id, car]: cars) {
        player_models[client_id] = car.get_model();
    }

    current_map_index++;
    race_with_countdown = race_total_time;
    results_time_remaining = results_screen_seconds;

    state = RaceState::ShowingResults;

    race = std::make_unique<RaceContext>(maps[current_map_index], registry);

    for (const auto& [client_id, model]: player_models) {
        race->spawn_car_for_player(client_id, model);
    }

    // Limpiamos estructura para las mejoras de los jugadores
    for (auto& [client_id, player]: players) {
        player.upgrade_received_this_round = false;
        player.pending_upgrade = 0;
    }
}

void Gameloop::race_finished() {
    if (state != RaceState::Running) {
        return;
    }

    auto& cars = race->get_cars();
    auto& rp_map = race->get_race_progress();
    std::vector<PlayerRaceResult> results;
    results.reserve(cars.size());
    add_all_results(cars, rp_map, results);
    bool is_last = (current_map_index + 1 >= maps.size());

    // Le pasamos ordenado los tiempos al cliente por resultados de la lobby entera
    std::sort(results.begin(), results.end(),
              [](const PlayerRaceResult& a, const PlayerRaceResult& b) {
                  if (a.total_time_seconds != b.total_time_seconds)
                      return a.total_time_seconds < b.total_time_seconds;
                  if (a.race_time_seconds != b.race_time_seconds)
                      return a.race_time_seconds < b.race_time_seconds;
                  return a.id < b.id;
              });

    if (is_last) {
        actual_result_time = 0.0;
        results_time_remaining = results_screen_seconds;
        state = RaceState::ShowingResultsLastRace;
        steps_result_table = 4;
        time_each_result_snapshot = results_screen_seconds / steps_result_table;
        result_snapshot_sent = 0;
        last_results = results;
        return;
    }

    race->send_race_results(results, is_last);

    create_new_race(cars);
}

void Gameloop::update_state_running(double dt) {
    race_with_countdown -= dt;
    if (race_with_countdown <= 0 || race->all_players_finished_or_dead()) {
        race_finished();
    }
}

void Gameloop::update_state_showing_results(double dt) {
    results_time_remaining -= dt;
    if (results_time_remaining <= 0.0 && current_map_index < maps.size()) {
        auto ev = std::make_shared<PhaseChangeEvent>();
        registry.broadcast(ev);
        results_time_remaining = upgrades_screen_seconds;
        state = RaceState::ChoosingUpgrades;
    }
}

void Gameloop::update_state_choosing_upgrades(double dt) {
    results_time_remaining -= dt;
    if (results_time_remaining <= 0.0) {
        auto ev = std::make_shared<PhaseChangeEvent>();
        registry.broadcast(ev);
        results_time_remaining = upgrades_screen_seconds;
        state = RaceState::WaitingUpgrades;
    }
}

void Gameloop::update_state_waiting_upgrades(double dt) {
    results_time_remaining -= dt;

    bool all_received = true;
    for (auto& [client_id, player]: players) {
        if (!player.upgrade_received_this_round) {
            all_received = false;
            break;
        }
    }

    if (all_received || results_time_remaining <= 0.0) {
        for (auto& [client_id, player]: players) {
            uint8_t up = player.pending_upgrade;
            uint8_t level = 0;
            if (up >= 1 && up <= 9) {
                level = static_cast<uint8_t>((up - 1) % 3 + 1);
            }
            double penalty = Config::instance().upgrade_penalty_for(level);
            player.total_time += penalty;

            if (up >= 1 && up <= 9) {
                player.applied_upgrades.push_back(up);
            }
        }

        start_new_map();
    }
}

void Gameloop::start_new_map() {
    // Antes de arrancar la nueva carrera (los autos ya han sido creados en el nuevo
    // mapa) los mejoramos con TODAS las mejoras que tuvieron hasta ahora!
    for (auto& [client_id, player]: players) {
        for (const auto& u: player.applied_upgrades) {
            race->upgrade_car(client_id, u);
        }
    }
    // Arrancamos la proxima carrera
    send_pre_game_snapshot();
    race_with_countdown = race_total_time;
    state = RaceState::Running;
    for (auto& [client_id, player]: players) {
        if (player.disconnected) {
            race->kill(client_id);
        }
    }
}

void Gameloop::update_state_showing_resultes_last_race(double dt) {
    results_time_remaining -= dt;
    actual_result_time += dt;

    while (result_snapshot_sent < steps_result_table &&
           time_each_result_snapshot * result_snapshot_sent <= actual_result_time) {

        race->send_race_results(last_results, true, result_snapshot_sent);

        result_snapshot_sent++;
    }

    if (results_time_remaining <= 0.0 && result_snapshot_sent >= steps_result_table) {
        state = RaceState::Finished;
        auto ev = std::make_shared<PhaseChangeEvent>();
        registry.broadcast(ev);
        try {
            // cdo quiera leer el prox comando, va a tirar excepcion y
            // se va a terminar el hilo del gameloop
            command_queue.close();
        } catch (...) {}
    }
}

void Gameloop::update_state(double dt) {
    if (state == RaceState::Running) {
        update_state_running(dt);
    } else if (state == RaceState::ShowingResults) {
        update_state_showing_results(dt);
    } else if (state == RaceState::ChoosingUpgrades) {
        update_state_choosing_upgrades(dt);
    } else if (state == RaceState::WaitingUpgrades) {
        update_state_waiting_upgrades(dt);
    } else if (state == RaceState::ShowingResultsLastRace) {
        update_state_showing_resultes_last_race(dt);
    }
}

void Gameloop::step_simulation(double& acumulate, double delta_time) {
    // Si nos atrasamos nos ponemos al dia
    while (acumulate >= delta_time) {
        if (state == RaceState::Running) {

            if ((race_total_time - race_with_countdown) >= race_countdown_time) {
                race->update_npcs();
            }

            // Aplicar inputs (estado "keys" -> fuerzas/torques del auto)
            race->apply_player_inputs();

            // Avanzar la física exactamente delta_time, con substeps para estabilidad
            race->step_physics();

            double race_with_countdown_actual = race_with_countdown;
            race->handle_race_and_contacts(race_with_countdown_actual);
        }
        acumulate -= delta_time;
    }
}

void Gameloop::send_snapshots(double& snapshot_acumulate, float snapshot_interval) {
    // Si nos atrasamos nos ponemos al dia
    while (snapshot_acumulate >= snapshot_interval) {
        if (state == RaceState::Running) {
            race->send_snapshot(snapshot_acumulate, snapshot_interval, race_with_countdown);
        } else {
            snapshot_acumulate -= snapshot_interval;
        }
    }
}

void Gameloop::run() {
    try {

        using clock = std::chrono::steady_clock;
        const float delta_time = race->get_time_step();

        auto t0 = clock::now();
        double acumulate = 0.0;
        double snapshot_acumulate = 0.0;

        const auto frame_duration = std::chrono::duration<double>(delta_time);

        while (should_keep_running()) {
            auto frame_start = clock::now();

            receive_commands();

            auto now = clock::now();
            std::chrono::duration<double> dt = now - t0;
            t0 = now;
            acumulate += dt.count();
            snapshot_acumulate += dt.count();

            update_state(dt.count());
            step_simulation(acumulate, delta_time);
            send_snapshots(snapshot_acumulate, delta_time);

            auto frame_end = clock::now();
            auto elapsed = frame_end - frame_start;

            if (elapsed < frame_duration) {
                std::this_thread::sleep_for(frame_duration - elapsed);
            }
        }

    } catch (const ClosedQueue&) {
        // Si la cola de comandos se cerro, salimos del gameloop
    } catch (const std::exception& e) {
        std::cerr << "Gameloop exception: " << e.what() << "\n";
    } catch (...) {
        std::cerr << "Gameloop unknown exception\n";
    }
}
