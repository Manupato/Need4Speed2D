#include "physic_world.h"

PhysicWorld::PhysicWorld(const std::string& path): map(path) {
    const auto& cfg = Config::instance();

    timeStep = cfg.physics_time_step();
    subSteps = cfg.physics_substeps();
    hitThreshold = cfg.hit_event_threshold();

    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2{0.0f, 0.0f};
    worldDef.hitEventThreshold = hitThreshold;
    worldId = b2CreateWorld(&worldDef);
}

void PhysicWorld::step() {
    // Avanzo la simulacion del mundo fisico
    b2World_Step(worldId, timeStep, subSteps);
}

void PhysicWorld::init_world() {
    map.create_static_colliders(worldId);
    map.create_checkpoint_sensors(worldId);
    map.create_bridge_sensors(worldId);
}

Spawn PhysicWorld::get_spawn_for_index(std::size_t idx) { return map.get_spawn_for_index(idx); }

const std::vector<Cell>& PhysicWorld::get_slow_cells() const { return map.get_slow_cells(); }

void PhysicWorld::handle_contacts() {

    b2ContactEvents ev = b2World_GetContactEvents(worldId);

    // Golpes
    for (int i = 0; i < ev.hitCount; ++i) {
        const b2ContactHitEvent* h = ev.hitEvents + i;

        // Filtramos solo choques de auto contra auto y auto contra la pared
        b2Filter fa = b2Shape_GetFilter(h->shapeIdA);
        b2Filter fb = b2Shape_GetFilter(h->shapeIdB);

        // Si fa.categoryBits == CAR -> distinto de 0 -> true
        bool aCar = (fa.categoryBits & (CAR_L0 | CAR_L1 | NPC_L0 | NPC_L1)) != 0;
        bool bCar = (fb.categoryBits & (CAR_L0 | CAR_L1 | NPC_L0 | NPC_L1)) != 0;
        bool aWall = (fa.categoryBits & (WALL_L0 | WALL_L1)) != 0;
        bool bWall = (fb.categoryBits & (WALL_L0 | WALL_L1)) != 0;

        // Ignoramos todo lo que no sea auto vs pared o auto vs auto
        if (!((aCar && (bCar || aWall || bWall)) || (bCar && (aCar || aWall || bWall))))
            continue;

        if (aCar) {
            // Obtenemos Car* desde el b2ShapeId (lo tenemos guardado en el UserData del bodyId)
            b2BodyId bid = b2Shape_GetBody(h->shapeIdA);
            Car* ca = static_cast<Car*>(b2Body_GetUserData(bid));
            // Si el auto termino, ya no recibe daño (fantasma)
            if (ca && !ca->is_finished()) {
                ca->apply_damage(h->approachSpeed);
            }
        }

        if (bCar) {
            b2BodyId bid = b2Shape_GetBody(h->shapeIdB);
            Car* cb = static_cast<Car*>(b2Body_GetUserData(bid));
            // Si el auto termino, ya no recibe daño (fantasma)
            if (cb && !cb->is_finished()) {
                cb->apply_damage(h->approachSpeed);
            }
        }
    }

    handle_bridge_contacts();
}

void PhysicWorld::handle_contact_in_sensor_bridge_i(std::unordered_set<Car*>& processed,
                                                    const int* targetZ, b2ShapeId& visitorId) {
    if (b2Shape_IsValid(visitorId) == false) {
        return;
    }

    // saco el body físico del que está tocando
    b2BodyId visitorBody = b2Shape_GetBody(visitorId);

    // leo el userData del body
    void* raw = b2Body_GetUserData(visitorBody);
    if (!raw) {
        return;
    }

    if ((b2Shape_GetFilter(visitorId).categoryBits & (CAR_L0 | CAR_L1)) == 0) {
        return;  // no es un auto
    }

    Car* car = static_cast<Car*>(raw);

    // Si ya lo procesamos en este frame, no lo tocamos de nuevo
    if (processed.count(car)) {
        return;
    }

    car->set_layer(*targetZ);
    processed.insert(car);
}

void PhysicWorld::handle_bridge_contacts() {
    const auto& bridge = map.get_bridge_sensors();

    // Autos procesados en este frame
    std::unordered_set<Car*> processed;

    for (b2ShapeId sensorShapeId: bridge) {

        if (!b2Shape_IsValid(sensorShapeId)) {
            std::cerr << "sensorShapeId invalido, lo salteo" << std::endl;
            continue;
        }

        int capacity = b2Shape_GetSensorCapacity(sensorShapeId);
        std::vector<b2ShapeId> overlaps;
        overlaps.resize(capacity);
        if (capacity == 0) {
            continue;
        }

        int count = b2Shape_GetSensorOverlaps(sensorShapeId, overlaps.data(), capacity);
        overlaps.resize(count);

        // Recuperar el targetZ desde el userData del cuerpo del sensor
        // EL mismo, nos dice si te sube o baja de nivel z
        b2BodyId sensorBody = b2Shape_GetBody(sensorShapeId);
        const int* targetZ = static_cast<int*>(b2Body_GetUserData(sensorBody));
        if (!targetZ)
            continue;

        for (int i = 0; i < count; ++i) {
            handle_contact_in_sensor_bridge_i(processed, targetZ, overlaps[i]);
        }
    }
}

MapId PhysicWorld::get_map_id() { return map.get_map_id(); }

PoleCoordsAndDirec PhysicWorld::get_pole_position() { return map.get_pole_position(); }

bool PhysicWorld::is_driveable_world_pos(float x, float y) const {
    int col = static_cast<int>(x);
    int row = map.getHeightInMeters() - 1 - static_cast<int>(y);

    if (col < 0 || row < 0 || col >= map.getWidthInMeters() || row >= map.getHeightInMeters()) {
        return false;
    }

    return map.is_driveable_cell(col, row);
}

std::vector<Spawn> PhysicWorld::get_npc_spawns_filtered(float min_dist_to_pole) const {
    return map.get_npc_spawns_filtered(min_dist_to_pole);
}

std::vector<Spawn> PhysicWorld::get_npc_park_spawns_filtered(float min_dist_to_pole) const {
    return map.get_npc_park_spawns_filtered(min_dist_to_pole);
}

PhysicWorld::~PhysicWorld() {
    // Destruyo el mundo de Box2D
    b2DestroyWorld(worldId);
}
