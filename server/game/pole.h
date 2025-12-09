#ifndef POLE_H
#define POLE_H

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "../event.h"

// Direccion de la pole
static constexpr uint8_t DERECHA = 0x01;
static constexpr uint8_t IZQUIERDA = 0x02;
static constexpr uint8_t ARRIBA = 0x03;
static constexpr uint8_t ABAJO = 0x04;
static constexpr float PI = 3.14159265358979323846f;

struct Spawn {
    // metros en MUNDO FISICO BOX2D
    float x;
    float y;
    float angle_rad;  // orientación inicial del auto
};

struct PoleCell {
    // columna y fila en LA MATRIZ LEIDA EN EL JSON
    int col;
    int row;
};

class Pole {
private:
    std::vector<PoleCell> poleCells;
    uint8_t direccion = DERECHA;

    // Alto total del mapa en CELDAS
    // Nos va a venir bien para hacer la conversion
    // De celdas a metros en Box2D (que es al reves!)
    int height = 0;

    // Desde donde vamos a calcular las posiciones
    PoleCell punta{0, 0};

    // Dependiendo de donde se arranque, la punta para calcular la pole va a ser diferente
    void set_punta_being_derecha();
    void set_punta_being_izquierda();
    void set_punta_being_abajo();
    void set_punta_being_arriba();

    // Dependiendo la punta, devolvemos el offset correspondiente:
    Spawn spawn_for_index_derecha(size_t index);
    Spawn spawn_for_index_izquierda(size_t index);
    Spawn spawn_for_index_abajo(size_t index);
    Spawn spawn_for_index_arriba(size_t index);

    float angle_for_direction(uint8_t direccion);

    // separacion entre filas (auto adelante vs auto atras)
    static constexpr float SLOT_FORWARD = 3.75f;
    // separacion lateral entre carriles
    static constexpr float SLOT_SIDE = 2.0f;
    // Geometría de la grilla / Box2D
    static constexpr float CELL_METERS = 1.0f;
    static constexpr float CELL_HALF = CELL_METERS / 2.0f;
    static constexpr float HALF = 0.5f;

public:
    Pole() = default;

    // Agregamos celdas de matriz del json
    void add_cell_to_pole(int x, int y);

    // Para un indice de auto (1 al 8) devuelve sus coords para q spawnee
    Spawn get_spawn_for_index(size_t index);

    // Direccion de la pole
    void set_direc(uint8_t dir);

    // Altura del mapa (la necesita para darle vuelta a las coords en y)
    void set_height_map(int h);

    PoleCoordsAndDirec get_pole_position();

    const std::vector<PoleCell>& get_pole_cells() const { return poleCells; }

    ~Pole() = default;
    Pole(const Pole&) = delete;
    Pole& operator=(const Pole&) = delete;
};

#endif  // POLE_H
