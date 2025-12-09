#ifndef PHYSIC_WORLD_H
#define PHYSIC_WORLD_H

#include <string>
#include <unordered_set>
#include <vector>

#include <box2d/box2d.h>

#include "../config.h"
#include "../event.h"

#include "car.h"
#include "map_loader.h"

class PhysicWorld {
private:
    // id del mundo de Box2D
    b2WorldId worldId;

    // Tiempo de paso para la simulacion
    float timeStep;
    // Cantidad de substeps para la simulacion
    int subSteps;
    float hitThreshold;

    MapLoader map;

    void handle_bridge_contacts();

    void handle_contact_in_sensor_bridge_i(std::unordered_set<Car*>& processed, const int* targetZ,
                                           b2ShapeId& visitorId);

public:
    // Se crea el mundo fisico con su respectiva configuracion
    explicit PhysicWorld(const std::string& path);

    void init_world();

    void step();

    b2WorldId getWorld() const { return worldId; }

    float getTimeStep() const { return timeStep; }

    Spawn get_spawn_for_index(std::size_t idx);

    int getHeightInMeters() const { return map.getHeightInMeters(); }

    const std::vector<CheckpointSensor>& get_sensors() const { return map.get_sensors(); }

    const std::vector<Cell>& get_slow_cells() const;

    void handle_contacts();

    MapId get_map_id();

    PoleCoordsAndDirec get_pole_position();

    bool is_driveable_world_pos(float x, float y) const;

    std::vector<Spawn> get_npc_spawns_filtered(float min_dist_to_pole) const;

    std::vector<Spawn> get_npc_park_spawns_filtered(float min_dist_to_pole) const;

    // Destruye el mundo fisico
    ~PhysicWorld();

    PhysicWorld(const PhysicWorld& other) = delete;
    PhysicWorld& operator=(const PhysicWorld& other) = delete;
};
#endif  // PHYSIC_WORLD_H
