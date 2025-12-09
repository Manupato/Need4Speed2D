#ifndef MAP_LOADER_H
#define MAP_LOADER_H

#include <fstream>
#include <list>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <box2d/box2d.h>
#include <nlohmann/json.hpp>

#include "../event.h"

#include "car.h"
#include "categories.h"
#include "checkpoint_sensor.h"
#include "pole.h"

struct Cell {
    int col;
    int row;
};

// Cada checkpoint va a tener varias celdas de sensores
// Y el orden es con el que hay que ir pasando los checkpoints
struct CheckpointDef {
    int order;
    std::vector<Cell> cells;
    bool goal = false;
};

struct NpcSpawnDef {
    int col;
    int row;
    NpcDir dir;
};

class MapLoader {
private:
    // Matriz que representa la grilla del mapa
    std::vector<std::vector<int>> grid;

    std::string base_map;

    // Dimensiones del mapa (en cantidad de celdas)
    int width;
    int height;

    // Todos los checkpoints de un mapa, ordenados por "order"
    std::vector<CheckpointDef> checkpoints;

    // El pasto, vereda o otras calles transitables pero que van a ir mas lentas
    std::vector<Cell> slow_cells;

    // Lista para almacenar los ordenes de los checkpoints en memoria estable
    std::list<int> orden;

    std::vector<CheckpointSensor> sensors;

    // Linea de largada de los jugadores
    Pole pole;

    void create_wall(uint32_t category_wall, uint32_t category_car, const b2WorldId& world, int x,
                     int y) const;

    std::vector<b2ShapeId> bridge_sensors;

    std::list<int> bridge_target_z_;

    std::vector<NpcSpawnDef> npc_spawns;
    std::vector<NpcSpawnDef> npc_spawns_park;

    void load_grid_and_dimensions(const nlohmann::json& j);
    void load_checkpoints(const nlohmann::json& j);
    void load_pole_direction(const nlohmann::json& j);
    void create_goal_pole_and_slow_zones();
    void load_npc_spawns_for_base_map(const nlohmann::json& j);

    std::vector<Spawn> filter_spawns_by_distance(const std::vector<NpcSpawnDef>& defs,
                                                 float min_dist_to_pole) const;

    /*
    Mapa: 290 x 292 celdas
    Cada celda: 16×16 px
    Física box2D: 1 celda = 1 metro
    factor PPM = 16 px / m
    */

    // Codigos de celda en la grilla del mapa
    static constexpr int CELL_WALL = 0;
    static constexpr int CELL_ROAD = 1;
    static constexpr int CELL_POLE = 2;
    static constexpr int CELL_GOAL = 3;
    static constexpr int CELL_SLOW = 5;
    static constexpr int CELL_WALL_L1_ONLY = 6;
    static constexpr int CELL_WALL_L0_ONLY = 8;
    static constexpr int CELL_BRIDGE_UP = 7;    // te levanta
    static constexpr int CELL_BRIDGE_DOWN = 9;  // te baja
    static constexpr int CELL_BRIDGE_UP_AND_POLE = 11;
    static constexpr int CELL_BRIDGE_DOWN_AND_POLE = 10;
    static constexpr int CELL_BRIDGE_UP_AND_GOAL = 13;
    static constexpr int CELL_BRIDGE_DOWN_AND_GOAL = 12;
    static constexpr int CELL_BRIDGE_UP_AND_CHECKPOINT = 15;
    static constexpr int CELL_BRIDGE_DOWN_AND_CHECKPOINT = 14;


    static constexpr float CELL_METERS = 1.0f;
    static constexpr float CELL_HALF = CELL_METERS / 2.0f;

    NpcDir dir_from_string(const std::string& s);


public:
    explicit MapLoader(const std::string& path);

    // Crea un cuerpo estatico por cada celda NO jugable
    void create_static_colliders(const b2WorldId& world) const;
    void create_checkpoint_sensors(const b2WorldId& world);
    void create_bridge_sensors(const b2WorldId& world);
    const std::vector<b2ShapeId>& get_bridge_sensors() const;

    int getHeightInMeters() const { return height; }

    int getWidthInMeters() const { return width; }

    const std::vector<CheckpointSensor>& get_sensors() const { return sensors; }

    Spawn get_spawn_for_index(std::size_t idx);

    const std::vector<Cell>& get_slow_cells() const;

    MapId get_map_id();

    PoleCoordsAndDirec get_pole_position();

    bool is_driveable_cell(int col, int row) const;

    std::vector<Spawn> get_npc_spawns_filtered(float min_dist_to_pole) const;

    std::vector<Spawn> get_npc_park_spawns_filtered(float min_dist_to_pole) const;

    ~MapLoader() = default;
    MapLoader(const MapLoader&) = delete;
    MapLoader& operator=(const MapLoader&) = delete;
};

#endif  // MAP_LOADER_H
