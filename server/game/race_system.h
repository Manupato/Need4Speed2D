#ifndef RACE_SYSTEM_H
#define RACE_SYSTEM_H

#include <iostream>
#include <vector>

#include "car.h"
#include "physic_world.h"
#include "world_state.h"

class RaceSystem {
private:
    PhysicWorld& physics;
    WorldState& world_state;

    void handle_contact_in_checkpoint_i(int i, double race_with_countdown_actual,
                                        std::vector<b2ShapeId>& overlaps, const CheckpointSensor& s,
                                        int checkpointOrder);

public:
    RaceSystem(PhysicWorld& physics, WorldState& world_state);

    void handle_checkpoint_contacts(double race_with_countdown_actual);
    bool all_players_finished_or_dead() const;

    ~RaceSystem() = default;
    RaceSystem(const RaceSystem& other) = delete;
    RaceSystem& operator=(const RaceSystem& other) = delete;
};

#endif  // RACE_SYSTEM_H
