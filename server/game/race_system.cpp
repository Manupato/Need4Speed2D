#include "race_system.h"

#include <vector>

RaceSystem::RaceSystem(PhysicWorld& physics, WorldState& world_state):
        physics(physics), world_state(world_state) {}

void RaceSystem::handle_checkpoint_contacts(double race_with_countdown_actual) {
    // Iteramos todos los sensores de checkpoint del mapa
    const auto& sens = physics.get_sensors();

    for (const auto& s: sens) {
        b2ShapeId sensorShapeId = s.get_shapeId();
        int checkpointOrder = *(s.get_orderPtr());

        int capacity = b2Shape_GetSensorCapacity(sensorShapeId);
        std::vector<b2ShapeId> overlaps;
        overlaps.resize(capacity);
        if (capacity == 0) {
            continue;
        }

        int count = b2Shape_GetSensorOverlaps(sensorShapeId, overlaps.data(), capacity);
        overlaps.resize(count);

        for (int i = 0; i < count; ++i) {
            handle_contact_in_checkpoint_i(i, race_with_countdown_actual, overlaps, s,
                                           checkpointOrder);
        }
    }
}

void RaceSystem::handle_contact_in_checkpoint_i(int i, double race_with_countdown_actual,
                                                std::vector<b2ShapeId>& overlaps,
                                                const CheckpointSensor& s, int checkpointOrder) {

    b2ShapeId visitorId = overlaps[i];

    if (b2Shape_IsValid(visitorId) == false) {
        return;
    }

    // saco el body físico del que está tocando
    b2BodyId visitorBody = b2Shape_GetBody(visitorId);

    // leo el userData del body
    // Los bodies de los checkpoints tienen userData = int* (storedOrder),
    // los autos tienen userData = Car*
    void* raw = b2Body_GetUserData(visitorBody);
    if (!raw || raw == s.get_orderPtr()) {
        return;
    }

    Car* car = reinterpret_cast<Car*>(raw);

    if (car->is_npc()) {
        return;
    }

    // necesitamos saber quien es el client_id que tiene este Car*
    int ownerClientId = world_state.get_owner_id(car);
    if (ownerClientId == -1) {
        return;
    }

    RaceProgress& prog = world_state.get_progress_of(ownerClientId);

    if (checkpointOrder == prog.next_order) {
        bool goal = s.is_goal();

        if (goal) {
            world_state.set_race_finish(ownerClientId, race_with_countdown_actual);

            auto& cars = world_state.get_cars();
            auto itCar = cars.find(ownerClientId);
            if (itCar != cars.end()) {
                Car& ownerCar = itCar->second;
                ownerCar.mark_finished();
                ownerCar.set_ghost(true);
            }
            prog.next_order++;
        } else {
            prog.next_order++;
        }
    }
}

bool RaceSystem::all_players_finished_or_dead() const {
    const auto& cars = world_state.get_cars();
    const auto& rp = world_state.get_race_progress();

    if (cars.empty()) {
        return false;
    }

    for (const auto& [player_id, car]: cars) {
        bool dead = car.is_destroyed();

        auto it = rp.find(player_id);
        bool finished = (it != rp.end() && it->second.time_remaining_when_finished > 0.0);

        // mientras haya alguien vivo y sin finish, la carrera sigue
        if (!dead && !finished) {
            return false;
        }
    }
    return true;
}
