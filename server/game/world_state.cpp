#include "world_state.h"

#include <cmath>
#include <cstdlib>
#include <ctime>

static constexpr float PI_LOCAL = 3.14159265358979323846f;

WorldState::WorldState(PhysicWorld& pw): pw(pw) {}

// Crea o devuelve el registro de teclas para ese jugador
teclas_presionadas& WorldState::inputs_for(int client_id) { return player_movements[client_id]; }

// Crea o devuelve el progreso para ese jugador
RaceProgress& WorldState::progress_for(int client_id) { return race_progress[client_id]; }

bool WorldState::client_have_car(const int client_id) const { return cars.count(client_id) != 0; }

void WorldState::change_w(bool new_state, const int client_id) {
    auto& keys = inputs_for(client_id);
    keys.w = new_state;
}

void WorldState::change_a(bool new_state, const int client_id) {
    auto& keys = inputs_for(client_id);
    keys.a = new_state;
}

void WorldState::change_s(bool new_state, const int client_id) {
    auto& keys = inputs_for(client_id);
    keys.s = new_state;
}

void WorldState::change_d(bool new_state, const int client_id) {
    auto& keys = inputs_for(client_id);
    keys.d = new_state;
}

std::size_t WorldState::number_of_players() const { return cars.size(); }

void WorldState::add_new_car(Spawn&& spawn, uint16_t new_car_model, int client_id,
                             b2WorldId worldId) {
    // seteamos el userData del body del auto
    // std::map::emplace(key, value) devuelve un std::pair<iterator, bool>:
    auto [it, inserted] =
            cars.emplace(client_id, Car(worldId, spawn.x, spawn.y, spawn.angle_rad, new_car_model));

    // (*it).first = client_id
    // (*it).second = Car&
    Car& newCar = it->second;
    newCar.set_user_data();

    player_movements[client_id] = teclas_presionadas{};
    race_progress[client_id] = RaceProgress{};
}

void WorldState::apply_player_inputs() {
    for (const auto& [client_id, keys]: player_movements) {
        auto car_it = cars.find(client_id);
        if (car_it != cars.end()) {
            Car& car = car_it->second;

            car.apply_input(keys.w, keys.s, keys.a, keys.d,
                            car.is_on_slow_zone(pw.get_slow_cells(), pw.getHeightInMeters()));
        }
    }
}

void WorldState::set_race_finish(int id_player, double time_finish) {
    auto& rp = progress_for(id_player);
    if (rp.time_remaining_when_finished == 0) {
        rp.time_remaining_when_finished = time_finish;
    }
}

void WorldState::win(int client_id, double race_with_countdown) {
    // Marcar fin de carrera para ese jugador
    set_race_finish(client_id, race_with_countdown);

    auto it = cars.find(client_id);
    if (it != cars.end()) {
        it->second.mark_finished();
        it->second.set_ghost(true);
    }
}

void WorldState::lose(int client_id) {
    auto it = cars.find(client_id);
    if (it != cars.end()) {
        it->second.kill();
    }
}

void WorldState::undestroyable(int client_id) {
    auto it = cars.find(client_id);
    if (it != cars.end()) {
        it->second.set_god_mode(true);
    }
}

void WorldState::ghost(int client_id) {
    auto it = cars.find(client_id);
    if (it != cars.end()) {
        it->second.set_ghost(!it->second.is_ghost());
    }
}

void WorldState::spawn_npc(Spawn&& spawn, uint16_t model, NpcDir dir, float speed) {
    npc_cars.emplace_back(pw.getWorld(), spawn.x, spawn.y, spawn.angle_rad, model);
    Car& car = npc_cars.back();
    car.set_user_data();
    car.make_npc(dir, speed);
}

void WorldState::update_npcs() {
    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        seeded = true;
    }

    for (Car& car: npc_cars) {

        if (!car.is_npc())
            continue;
        if (car.is_destroyed())
            continue;

        NpcState& st = car.npc_state();

        // Es un npc estacionado, no lo movemos
        if (st.speed == 0.0)
            continue;

        st.steps_since_last_turn++;

        const NpcDir forward = st.dir;
        const NpcDir left = left_of(st.dir);
        const NpcDir right = right_of(st.dir);
        const NpcDir back = opposite_of(st.dir);

        const bool can_fwd = can_go(car, forward);
        const bool can_left = can_go(car, left);
        const bool can_right = can_go(car, right);
        const bool can_back = can_go(car, back);

        //  Todas las options, las metemos en un vector
        std::vector<NpcDir> options;

        if (can_fwd)
            options.push_back(forward);
        if (can_left)
            options.push_back(left);
        if (can_right)
            options.push_back(right);

        NpcDir new_dir = st.dir;

        if (st.steps_since_last_turn > MIN_STEPS_BETWEEN_TURNS) {
            if (!options.empty()) {
                new_dir = options[std::rand() % options.size()];
            } else if (can_back) {
                // Nunca priorizamos ir hacia atras
                new_dir = back;
            } else {
                car.kill();
                continue;
            }
        } else {
            // Si no paso el minimo para doblar, y podemos seguir adelante, seguimos adelante
            if (can_fwd) {
                new_dir = st.dir;
            } else if (!options.empty()) {
                new_dir = options[std::rand() % options.size()];
            } else if (can_back) {
                // Nunca priorizamos ir hacia atras
                new_dir = back;
            } else {
                car.kill();
                continue;
            }
        }

        if (new_dir != st.dir) {
            st.dir = new_dir;

            b2Vec2 pos = car.get_position();
            float angle = angle_for_npc_dir(st.dir);
            car.force_set_transform(pos.x, pos.y, angle);
            st.steps_since_last_turn = 0;
        }

        if (!car.is_destroyed()) {
            car.force_set_forward_speed(st.speed);
        }
    }
}

bool WorldState::can_go(const Car& car, NpcDir dir) {
    int METERS_FORWARD = 2;
    if (car.npc_state().dir != dir) {
        METERS_FORWARD = 5;
    }

    const b2Vec2 pos = car.get_position();
    const float angle = angle_for_npc_dir(dir);

    for (int c = 1; c <= METERS_FORWARD; ++c) {
        float d = static_cast<float>(c);
        float x = pos.x + std::cos(angle) * d;
        float y = pos.y + std::sin(angle) * d;
        /*
        cos(0) = 1; sin(0) = 0
        cos(PI/2) = 0; sin(PI/2) = 1
        cos(PI) = -1; sin(PI) = 0
        cos(-PI/2) = 0; sin(-PI/2) = -1
        */

        if (!pw.is_driveable_world_pos(x, y)) {
            return false;
        }
    }

    if (dir == NpcDir::Right || dir == NpcDir::Left) {
        for (int c = 1; c <= METERS_FORWARD; ++c) {
            float d = static_cast<float>(c);
            float x = pos.x + std::cos(angle) * d;
            float y = pos.y + 1 + std::sin(angle) * d;
            if (!pw.is_driveable_world_pos(x, y)) {
                return false;
            }
        }
        for (int c = 1; c <= METERS_FORWARD; ++c) {
            float d = static_cast<float>(c);
            float x = pos.x + std::cos(angle) * d;
            float y = pos.y - 1 + std::sin(angle) * d;
            if (!pw.is_driveable_world_pos(x, y)) {
                return false;
            }
        }
    }

    if (dir == NpcDir::Up || dir == NpcDir::Down) {
        for (int c = 1; c <= METERS_FORWARD; ++c) {
            float d = static_cast<float>(c);
            float x = pos.x + 1 + std::cos(angle) * d;
            float y = pos.y + std::sin(angle) * d;
            if (!pw.is_driveable_world_pos(x, y)) {
                return false;
            }
        }
        for (int c = 1; c <= METERS_FORWARD; ++c) {
            float d = static_cast<float>(c);
            float x = pos.x - 1 + std::cos(angle) * d;
            float y = pos.y + std::sin(angle) * d;
            if (!pw.is_driveable_world_pos(x, y)) {
                return false;
            }
        }
    }
    return true;
}

float WorldState::angle_for_npc_dir(NpcDir dir) {
    switch (dir) {
        case NpcDir::Right:
            return 0.0f;
        case NpcDir::Up:
            return (0.5f * PI_LOCAL);
        case NpcDir::Left:
            return PI_LOCAL;
        case NpcDir::Down:
            return (-0.5f * PI_LOCAL);
    }
    return 0.0f;
}

NpcDir WorldState::left_of(NpcDir dir) {
    switch (dir) {
        case NpcDir::Up:
            return NpcDir::Left;
        case NpcDir::Left:
            return NpcDir::Down;
        case NpcDir::Down:
            return NpcDir::Right;
        case NpcDir::Right:
            return NpcDir::Up;
    }
    return dir;
}

NpcDir WorldState::right_of(NpcDir dir) {
    switch (dir) {
        case NpcDir::Up:
            return NpcDir::Right;
        case NpcDir::Right:
            return NpcDir::Down;
        case NpcDir::Down:
            return NpcDir::Left;
        case NpcDir::Left:
            return NpcDir::Up;
    }
    return dir;
}

NpcDir WorldState::opposite_of(NpcDir dir) {
    switch (dir) {
        case NpcDir::Up:
            return NpcDir::Down;
        case NpcDir::Down:
            return NpcDir::Up;
        case NpcDir::Left:
            return NpcDir::Right;
        case NpcDir::Right:
            return NpcDir::Left;
    }
    return dir;
}

int WorldState::get_owner_id(const Car* car) const {
    for (const auto& [cid, c]: cars) {
        if (&c == car) {
            return cid;
        }
    }
    return -1;
}
