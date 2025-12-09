#include "pole.h"

#include <iostream>

void Pole::add_cell_to_pole(int x, int y) { poleCells.emplace_back(PoleCell{x, y}); }

void Pole::set_direc(uint8_t dir) { direccion = dir; }

void Pole::set_height_map(int h) { height = h; }

Spawn Pole::get_spawn_for_index(size_t index) {
    switch (direccion) {
        case DERECHA:
            set_punta_being_derecha();
            return spawn_for_index_derecha(index);
        case IZQUIERDA:
            set_punta_being_izquierda();
            return spawn_for_index_izquierda(index);
        case ARRIBA:
            set_punta_being_arriba();
            return spawn_for_index_arriba(index);
        case ABAJO:
            set_punta_being_abajo();
            return spawn_for_index_abajo(index);
        default:
            throw std::runtime_error("MapLoader: Direccion de la pole desconocida");
    }
    return Spawn{0.0f, 0.0f, 0.0f};
}

void Pole::set_punta_being_derecha() {
    // Si la direccion es la derecha, la posicion 1 seria la x mas grande y
    // la y mas chica (pq la matriz esta invertida por ahora!)
    auto it = std::max_element(poleCells.begin(), poleCells.end(),
                               [](const PoleCell& a, const PoleCell& b) {
                                   // Queremos el que tenga col mas grande
                                   if (a.col != b.col)
                                       return a.col < b.col;  // el "mayor" tiene col mas grande
                                   // Si col es igual, queremos el de menor row
                                   return a.row > b.row;  // el "mayor" tiene row mas chico
                               });
    if (it == poleCells.end()) {
        throw std::runtime_error("MapLoader: Pole erronea");
    }
    punta = *it;
}

void Pole::set_punta_being_izquierda() {
    // Si la direccion es la izq, la posicion 1 seria la x mas chica y
    // la y mas chica (pq la matriz esta invertida por ahora!)
    auto it = std::max_element(poleCells.begin(), poleCells.end(),
                               [](const PoleCell& a, const PoleCell& b) {
                                   // Queremos el que tenga col mas grande
                                   if (a.col != b.col)
                                       return a.col > b.col;  // el "mayor" tiene col mas chica
                                   // Si col es igual, queremos el de menor row
                                   return a.row > b.row;  // el "mayor" tiene row mas chico
                               });
    if (it == poleCells.end()) {
        throw std::runtime_error("MapLoader: Pole erronea");
    }
    punta = *it;
}

void Pole::set_punta_being_abajo() {
    // Si la direccion es abajo, la posicion 1 seria la x mas chica y
    // la y mas grande (pq la matriz esta invertida por ahora!)
    auto it = std::max_element(poleCells.begin(), poleCells.end(),
                               [](const PoleCell& a, const PoleCell& b) {
                                   // Queremos el que tenga col mas grande
                                   if (a.col != b.col)
                                       return a.col > b.col;  // el "mayor" tiene col mas grande
                                   // Si col es igual, queremos el de menor row
                                   return a.row < b.row;  // el "mayor" tiene row mas grande
                               });
    if (it == poleCells.end()) {
        throw std::runtime_error("MapLoader: Pole erronea");
    }
    punta = *it;
}

void Pole::set_punta_being_arriba() {
    // Si la direccion es arriba, la posicion 1 seria la x mas grande y
    // la y mas chica (pq la matriz esta invertida por ahora!)
    auto it = std::max_element(poleCells.begin(), poleCells.end(),
                               [](const PoleCell& a, const PoleCell& b) {
                                   // Queremos el que tenga col mas grande
                                   if (a.col != b.col)
                                       return a.col < b.col;  // el "mayor" tiene col mas grande
                                   // Si col es igual, queremos el de menor row
                                   return a.row > b.row;  // el "mayor" tiene row mas grande
                               });
    if (it == poleCells.end()) {
        throw std::runtime_error("MapLoader: Pole erronea");
    }
    punta = *it;
}


/*
Por cada auto tenemos 2m (SLOT_SIDE) de ancho y 3.75m (SLOT_FORWARD) de largo. Los queremos
posicionar en el medio
-> + SLOT_SIDE/2, + SLOT_FORWARD/2

Por lo tanto, dependiendo su indice (y la direccion de la pole) se le va a sumar n veces el slot
para ubicarlo!
*/
Spawn Pole::spawn_for_index_derecha(size_t index) {
    // Calculamos la posicion segun el indice del auto
    int fila = static_cast<int>(index / 2);    // 0..3
    int carril = static_cast<int>(index % 2);  // 0..1

    // Convertimos la celda 'punta' a coordenadas fisicas Box2D EN EL CENTRO de esa celda
    float base_x = static_cast<float>(punta.col) + CELL_HALF;
    float base_y = static_cast<float>(height - 1 - punta.row) + CELL_METERS;

    //  - La primera fila (fila=0) tiene que estar mas ADELANTE en la direccion de carrera.
    //    Direccion de carrera = +X.
    //    Si vamos hacia la derecha, "atras" es hacia -X.
    //    cada fila mas atras se corre -SLOT_FORWARD en X.
    //
    //    Ademas queremos que el auto quede al centro del slot
    //    -> restar SLOT_FORWARD/2 desde la punta
    //
    float x = base_x - (SLOT_FORWARD * HALF)  // centro del primer slot longitudinal
              - (fila * SLOT_FORWARD);        // filas siguientes mas atras todavia (-X)


    // Misma logica que en x, para y

    float y = base_y - (SLOT_SIDE * HALF) - (carril * SLOT_SIDE);

    return Spawn{x, y, angle_for_direction(DERECHA)};
}

Spawn Pole::spawn_for_index_izquierda(size_t index) {
    int fila = static_cast<int>(index / 2);
    int carril = static_cast<int>(index % 2);

    float base_x = static_cast<float>(punta.col) + CELL_HALF;
    float base_y = static_cast<float>(height - 1 - punta.row) + CELL_METERS;

    float x = base_x + (SLOT_FORWARD * HALF) + (fila * SLOT_FORWARD);

    float y = base_y - (SLOT_SIDE * HALF) - (carril * SLOT_SIDE);

    return Spawn{x, y, angle_for_direction(IZQUIERDA)};
}

Spawn Pole::spawn_for_index_abajo(size_t index) {
    int fila = static_cast<int>(index / 2);
    int carril = static_cast<int>(index % 2);

    float base_x = static_cast<float>(punta.col);
    float base_y = static_cast<float>(height - 1 - punta.row) + CELL_HALF;

    float y = base_y + (SLOT_FORWARD * HALF) + (fila * SLOT_FORWARD);

    float x = base_x + (SLOT_SIDE * HALF) + (carril * SLOT_SIDE);

    return Spawn{x, y, angle_for_direction(ABAJO)};
}

Spawn Pole::spawn_for_index_arriba(size_t index) {
    int fila = static_cast<int>(index / 2);
    int carril = static_cast<int>(index % 2);

    float base_x = static_cast<float>(punta.col) - CELL_METERS;
    float base_y = static_cast<float>(height - 1 - punta.row) + CELL_HALF;

    float y = base_y - (SLOT_FORWARD * HALF) - (fila * SLOT_FORWARD);

    float x = base_x - (SLOT_SIDE * HALF) + (carril * SLOT_SIDE);

    return Spawn{x, y, angle_for_direction(ARRIBA)};
}


float Pole::angle_for_direction(uint8_t direccion) {
    switch (direccion) {
        case DERECHA:
            return 0.0f;  // mira +X
        case IZQUIERDA:
            return PI;  // mira -X
        case ARRIBA:
            return 0.5f * PI;  // mira +Y físico
        case ABAJO:
            return -0.5f * PI;  // mira -Y físico
        default:
            return 0.0f;
    }
}

// La posicion de la pole en px para que la pueda mostrar el cliente sin hacer conversiones raras
PoleCoordsAndDirec Pole::get_pole_position() {
    int minCol = poleCells[0].col;
    int maxCol = minCol;
    int minRow = poleCells[0].row;
    int maxRow = minRow;

    for (const auto& cell: poleCells) {
        minCol = std::min(minCol, cell.col);
        maxCol = std::max(maxCol, cell.col);
        minRow = std::min(minRow, cell.row);
        maxRow = std::max(maxRow, cell.row);
    }

    Coord coord_up_left = {static_cast<uint32_t>((minCol)*16.0f),
                           static_cast<uint32_t>((minRow)*16.0f)};
    Coord coord_down_right = {static_cast<uint32_t>((maxCol + 1) * 16.0f),
                              static_cast<uint32_t>((maxRow + 1) * 16.0f)};
    uint8_t direc = direccion;

    return (PoleCoordsAndDirec{coord_up_left, coord_down_right, direc});
}
