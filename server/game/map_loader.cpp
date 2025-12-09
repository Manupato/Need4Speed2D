#include "map_loader.h"

#include <algorithm>
#include <random>

MapLoader::MapLoader(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("MapLoader: No se puede abrir el archivo " + path);
    }

    nlohmann::json j;
    file >> j;

    load_grid_and_dimensions(j);
    load_checkpoints(j);
    load_pole_direction(j);
    create_goal_pole_and_slow_zones();
    load_npc_spawns_for_base_map(j);
}

void MapLoader::load_grid_and_dimensions(const nlohmann::json& j) {
    nlohmann::json grid_json;

    if (j.is_object()) {
        if (!j.contains("grid"))
            throw std::runtime_error("MapLoader: falta la clave 'grid' en el objeto JSON");
        grid_json = j["grid"];

        if (j.contains("base_map"))
            base_map = j["base_map"].get<std::string>();
    } else if (j.is_array()) {
        grid_json = j;
    } else {
        throw std::runtime_error("MapLoader: se esperaba objeto con 'grid' o matriz [[...], ...]");
    }

    grid = grid_json.get<std::vector<std::vector<int>>>();

    if (grid.empty() || grid[0].empty()) {
        throw std::runtime_error("MapLoader: matriz vacía");
    }

    height = static_cast<int>(grid.size());
    width = static_cast<int>(grid[0].size());

    bool rectangular = std::all_of(grid.begin(), grid.end(), [this](const auto& row) {
        return static_cast<int>(row.size()) == this->width;
    });

    if (!rectangular) {
        throw std::runtime_error("MapLoader: Matriz no rectangular");
    }
}

void MapLoader::load_checkpoints(const nlohmann::json& j) {
    if (j.contains("checkpoints_order") && j["checkpoints_order"].is_array()) {
        for (const auto& cp: j["checkpoints_order"]) {
            CheckpointDef def;
            def.order = cp.value("order", 0);
            for (const auto& c: cp["cells"]) {
                def.cells.push_back({c.value("col", 0), c.value("row", 0)});
            }
            checkpoints.push_back(std::move(def));
        }
        std::sort(checkpoints.begin(), checkpoints.end(),
                  [](const auto& a, const auto& b) { return a.order < b.order; });
    }
}

void MapLoader::load_pole_direction(const nlohmann::json& j) {
    if (j.contains("direccion_salida")) {
        std::string direccion_salida = j["direccion_salida"].get<std::string>();
        if (direccion_salida == "derecha") {
            pole.set_direc(DERECHA);
        } else if (direccion_salida == "izquierda") {
            pole.set_direc(IZQUIERDA);
        } else if (direccion_salida == "arriba") {
            pole.set_direc(ARRIBA);
        } else if (direccion_salida == "abajo") {
            pole.set_direc(ABAJO);
        } else {
            throw std::runtime_error("MapLoader: La direccion de pole de salida es incorrecta");
        }
    } else {
        throw std::runtime_error("MapLoader: La pole de salida no tiene direccion");
    }
}

void MapLoader::create_goal_pole_and_slow_zones() {
    std::vector<Cell> goalCells;
    // Como mucho, son 9
    goalCells.reserve(9);

    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            if (grid[row][col] == CELL_POLE || grid[row][col] == CELL_BRIDGE_UP_AND_POLE ||
                grid[row][col] == CELL_BRIDGE_DOWN_AND_POLE) {
                pole.add_cell_to_pole(col, row);
            } else if (grid[row][col] == CELL_SLOW) {
                slow_cells.emplace_back(Cell{col, row});
            } else if (grid[row][col] == CELL_GOAL || grid[row][col] == CELL_BRIDGE_UP_AND_GOAL ||
                       grid[row][col] == CELL_BRIDGE_DOWN_AND_GOAL) {
                goalCells.push_back(Cell{col, row});
            }
        }
    }

    pole.set_height_map(height);

    if (goalCells.empty()) {
        return;
    }

    // Calculamos el orden que le toca a la meta
    int maxOrder = 0;
    if (!checkpoints.empty()) {
        maxOrder = checkpoints.back().order;
    }

    CheckpointDef goalDef;
    goalDef.order = maxOrder + 1;
    goalDef.cells = std::move(goalCells);
    goalDef.goal = true;

    checkpoints.push_back(std::move(goalDef));
}

NpcDir MapLoader::dir_from_string(const std::string& s) {
    if (s == "Right") {
        return NpcDir::Right;
    }
    if (s == "Left") {
        return NpcDir::Left;
    }
    if (s == "Up") {
        return NpcDir::Up;
    }
    if (s == "Down") {
        return NpcDir::Down;
    }
    throw std::runtime_error("MapLoader: direccion de NPC invalida: " + s);
}

// Algunos spawns para los npcs. De querer mas, se pueden agregar aca!
void MapLoader::load_npc_spawns_for_base_map(const nlohmann::json& j) {
    npc_spawns.clear();
    npc_spawns_park.clear();

    // npc_spawns normales
    if (j.contains("npc_spawns") && j["npc_spawns"].is_array()) {
        for (const auto& s: j["npc_spawns"]) {
            NpcSpawnDef def;
            def.col = s.value("col", 0);
            def.row = s.value("row", 0);

            std::string dir_str = s.value("dir", "Right");
            def.dir = dir_from_string(dir_str);

            npc_spawns.push_back(def);
        }
    }

    // npc_spawns_park
    if (j.contains("npc_spawns_park") && j["npc_spawns_park"].is_array()) {
        for (const auto& s: j["npc_spawns_park"]) {
            NpcSpawnDef def;
            def.col = s.value("col", 0);
            def.row = s.value("row", 0);

            std::string dir_str = s.value("dir", "Right");
            def.dir = dir_from_string(dir_str);

            npc_spawns_park.push_back(def);
        }
    }

    // Mezclar los vectores para que el orden sea aleatorio
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(npc_spawns.begin(), npc_spawns.end(), g);
    std::shuffle(npc_spawns_park.begin(), npc_spawns_park.end(), g);
}

// Coordenadas en METROS: cada celda es 1x1 METRO
// El centro del cuerpo está en el centro de la celda
// Por ejemplo, celda (0,0) va de (0,0) a (1,1) en metros.
// Y su centro está en (0.5, 0.5).
// En Box2D la altura y ancho es desde el centro.
// Entonces el box 1x1m tiene 0.5m de ancho y 0.5m de alto.

// Centro del box en (x+0.5, y+0.5)
// En Y, hay que invertir la coordenada porque el mundo del juego
// tiene el origen en la esquina inferior izquierda, pero la matriz
// del mapa tiene el origen en la esquina superior izquierda.
// Entonces si y=0 en la matriz, en el mundo es y = height - 1 - 0.
// si y=1 en la matriz, en el mundo es y = height - 1 - 1, etc.
// Notar que asi, estamos poniendo en el mismo sentido la matriz y el mundo.
// Esto es opcional igualmente, porque el cliente va a ser el que dibuje
// el mapa y puede hacer la conversión que quiera. Pero me ayuda a ahorrarme
// dolores de cabeza.

void MapLoader::create_wall(uint32_t category_wall, uint32_t category_car, const b2WorldId& world,
                            int x, int y) const {
    b2BodyDef groundBodyDef = b2DefaultBodyDef();
    groundBodyDef.position = b2Vec2{static_cast<float>(x) + CELL_HALF,
                                    static_cast<float>(height - 1 - y) + CELL_HALF};
    b2BodyId groundId = b2CreateBody(world, &groundBodyDef);

    b2Polygon box = b2MakeBox(CELL_HALF, CELL_HALF);
    b2ShapeDef sdef = b2DefaultShapeDef();

    sdef.enableHitEvents = true;

    sdef.filter.categoryBits = category_wall;
    sdef.filter.maskBits = category_car;

    sdef.material.friction = 1.2f;
    sdef.material.restitution = 0.0f;
    b2CreatePolygonShape(groundId, &sdef, &box);
}

void MapLoader::create_static_colliders(const b2WorldId& world) const {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (grid[y][x] == CELL_WALL) {  // Si es pared
                create_wall(BOTH_WALLS, BOTH_CARS, world, x, y);
            } else if (grid[y][x] == CELL_WALL_L1_ONLY) {
                // Se pueden transitar en altura 0, pero en 1 bloquean
                create_wall(WALL_L1, CAR_L1, world, x, y);
            } else if (grid[y][x] == CELL_WALL_L0_ONLY) {
                // dejan el paso en altura 1 pero en altura 0 bloquean
                create_wall(WALL_L0, CAR_L0, world, x, y);
            }
        }
    }

    // Por las dudas, agrego paredes a mano en todos los bordes
    // Fila de arriba y de abajo
    for (int x = 0; x < width; ++x) {
        create_wall(BOTH_WALLS, BOTH_CARS, world, x, 0);
        create_wall(BOTH_WALLS, BOTH_CARS, world, x, height - 1);
    }

    // Columna izquierda y derecha
    for (int y = 0; y < height; ++y) {
        create_wall(BOTH_WALLS, BOTH_CARS, world, 0, y);
        create_wall(BOTH_WALLS, BOTH_CARS, world, width - 1, y);
    }
}

void MapLoader::create_checkpoint_sensors(const b2WorldId& world) {
    for (const auto& cp: checkpoints) {

        // insertamos y obtenemos referencia estable al int dentro de la lista
        auto& storedOrder = orden.emplace_back(cp.order);

        for (const auto& cell: cp.cells) {
            const int x = cell.col;
            const int y = cell.row;

            b2BodyDef def = b2DefaultBodyDef();  // static por defecto
            def.position = b2Vec2{static_cast<float>(x) + CELL_HALF,
                                  static_cast<float>(height - 1 - y) + CELL_HALF};

            // guardamos el 'order' en el body userData para reconocerlo desde los contactos
            def.userData = &storedOrder;

            b2BodyId body = b2CreateBody(world, &def);

            b2Polygon box = b2MakeBox(CELL_HALF, CELL_HALF);
            b2ShapeDef sdef = b2DefaultShapeDef();
            sdef.isSensor = true;
            sdef.enableSensorEvents = true;
            sdef.filter.categoryBits = SENSOR;
            sdef.filter.maskBits = CAR_L0 | CAR_L1;  // Los sensores son para ambas capas

            // No se choca ni rebota con el sensor
            sdef.material.friction = 0.0f;
            sdef.material.restitution = 0.0f;

            b2ShapeId shapeId = b2CreatePolygonShape(body, &sdef, &box);

            sensors.push_back(CheckpointSensor(shapeId, &storedOrder, x, y, cp.goal));
        }
    }
}

// Creamos los sensores que te elevan y bajan de un puente
void MapLoader::create_bridge_sensors(const b2WorldId& world) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            if (grid[y][x] != CELL_BRIDGE_UP && grid[y][x] != CELL_BRIDGE_DOWN &&
                grid[y][x] != CELL_BRIDGE_UP_AND_POLE && grid[y][x] != CELL_BRIDGE_DOWN_AND_POLE &&
                grid[y][x] != CELL_BRIDGE_UP_AND_GOAL && grid[y][x] != CELL_BRIDGE_DOWN_AND_GOAL &&
                grid[y][x] != CELL_BRIDGE_UP_AND_CHECKPOINT &&
                grid[y][x] != CELL_BRIDGE_DOWN_AND_CHECKPOINT)
                continue;

            // Puntero estable porque es lista
            bool is_up = (grid[y][x] == CELL_BRIDGE_UP || grid[y][x] == CELL_BRIDGE_UP_AND_POLE ||
                          grid[y][x] == CELL_BRIDGE_UP_AND_GOAL ||
                          grid[y][x] == CELL_BRIDGE_UP_AND_CHECKPOINT);
            bridge_target_z_.push_back(is_up ? 1 : 0);
            int* targetZ = &bridge_target_z_.back();

            b2BodyDef def = b2DefaultBodyDef();
            def.position = b2Vec2{static_cast<float>(x) + CELL_HALF,
                                  static_cast<float>(height - 1 - y) + CELL_HALF};

            // En el userData del sensor, guardamos si te sube o baja de nivel
            def.userData = targetZ;

            b2BodyId body = b2CreateBody(world, &def);
            b2Polygon box = b2MakeBox(CELL_HALF, CELL_HALF);
            b2ShapeDef sdef = b2DefaultShapeDef();
            sdef.isSensor = true;
            sdef.enableSensorEvents = true;
            sdef.filter.categoryBits = SENSOR;
            sdef.filter.maskBits = CAR_L0 | CAR_L1 | NPC_L0 | NPC_L1;
            sdef.material.friction = 0.0f;
            sdef.material.restitution = 0.0f;
            b2ShapeId shapeId = b2CreatePolygonShape(body, &sdef, &box);

            bridge_sensors.emplace_back(shapeId);
        }
    }
}

Spawn MapLoader::get_spawn_for_index(std::size_t idx) { return pole.get_spawn_for_index(idx); }

const std::vector<Cell>& MapLoader::get_slow_cells() const { return slow_cells; }

const std::vector<b2ShapeId>& MapLoader::get_bridge_sensors() const { return bridge_sensors; }

MapId MapLoader::get_map_id() {
    if (base_map == "ViceCity.png") {
        return MapId::ViceCity;
    } else if (base_map == "LibertyCity.png") {
        return MapId::LibertyCity;
    } else if (base_map == "SanAndreas.png") {
        return MapId::SanAndreas;
    }

    throw std::runtime_error("MapLoader: No se encontro el id del mapa" + base_map);
}

PoleCoordsAndDirec MapLoader::get_pole_position() { return pole.get_pole_position(); }

bool MapLoader::is_driveable_cell(int col, int row) const {
    if (row < 0 || row >= height || col < 0 || col >= width)
        return false;
    int v = grid[row][col];

    return (v == CELL_ROAD || v == CELL_POLE || v == CELL_GOAL || v == CELL_BRIDGE_UP ||
            v == CELL_BRIDGE_DOWN || v == CELL_BRIDGE_DOWN_AND_POLE ||
            v == CELL_BRIDGE_UP_AND_POLE || v == CELL_BRIDGE_UP_AND_GOAL ||
            v == CELL_BRIDGE_DOWN_AND_GOAL || v == CELL_BRIDGE_UP_AND_CHECKPOINT ||
            v == CELL_BRIDGE_DOWN_AND_CHECKPOINT || v == CELL_WALL_L1_ONLY);
}

std::vector<Spawn> MapLoader::filter_spawns_by_distance(const std::vector<NpcSpawnDef>& defs,
                                                        float min_dist_to_pole) const {
    std::vector<Spawn> result;
    const auto& pole_cells = pole.get_pole_cells();

    for (const auto& def: defs) {
        bool too_close = false;

        for (const auto& pc: pole_cells) {
            int dx = def.col - pc.col;
            int dy = def.row - pc.row;
            float dist2 = float(dx * dx + dy * dy);

            // distancia euclidea (d = raiz(dx^2 + dy^2))
            if (dist2 < min_dist_to_pole * min_dist_to_pole) {
                too_close = true;
                break;
            }
        }
        if (too_close)
            continue;

        float x = static_cast<float>(def.col) + CELL_HALF;
        float y = static_cast<float>(height - 1 - def.row) + CELL_HALF;

        float angle = 0.0f;
        switch (def.dir) {
            case NpcDir::Right:
                angle = 0.0f;
                break;
            case NpcDir::Up:
                angle = 0.5f * PI;
                break;
            case NpcDir::Left:
                angle = PI;
                break;
            case NpcDir::Down:
                angle = 1.5f * PI;
                break;
        }

        result.push_back(Spawn{x, y, angle});
    }

    return result;
}

std::vector<Spawn> MapLoader::get_npc_spawns_filtered(float min_dist_to_pole) const {
    return filter_spawns_by_distance(npc_spawns, min_dist_to_pole);
}

std::vector<Spawn> MapLoader::get_npc_park_spawns_filtered(float min_dist_to_pole) const {
    return filter_spawns_by_distance(npc_spawns_park, min_dist_to_pole);
}
