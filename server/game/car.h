#ifndef CAR_H
#define CAR_H

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <map>
#include <vector>

#include <box2d/box2d.h>

#include "car_design.h"
#include "categories.h"

struct Cell;

enum class NpcDir { Right, Left, Up, Down };

struct NpcState {
    // false = no es NPC (auto de jugador)
    bool active = false;
    NpcDir dir{};
    float speed = 0.0f;
    int steps_since_last_turn = 0;
};

enum class UpgradesOfACar : uint8_t {
    Nothing = 0,
    Vel1 = 1,
    Vel2 = 2,
    Vel3 = 3,
    Shield1 = 4,
    Shield2 = 5,
    Shield3 = 6,
    Handle1 = 7,
    Handle2 = 8,
    Handle3 = 9
};

struct CarParams {
    // Fuerza que impulsa el auto hacia adelante. Aceleracion
    float engineForce;

    // Que tan rapido el auto gira. Valores mas altos lo hacen inmanejable
    float turnTorque;

    // Velocidad maxima del auto.
    float maxSpeed;

    // Dimensiones FISICAS EN EL JUEGO. 16px en fotos = 1 metro en box2D
    float length;  // eje X local = largo
    float width;   // eje Y local = ancho
    // Esto teniendo en cuenta la foto 0 de los assets

    // Masa = densidad * area. A mayor densidad, el auto será mas pesado
    float density;

    // Como el auto se desliza sobre superficies. Un valor mas alto, se desliza menos.
    float friction;

    float shield;
};

class Car {
private:
    static constexpr float MIN_FORWARD_SPEED_FOR_BRAKE = 6.0f;
    static constexpr float MAX_ANGULAR_VEL = 2.0f;
    static constexpr float DRAG_COEFF = 0.8f;
    static constexpr float EXTRA_ANGULAR_DAMPING = 2.5f;

    static constexpr float CRASH_SPEED_LOW = 8.0f;
    static constexpr float CRASH_SPEED_MED = 12.0f;

    static constexpr int DESTROY_EVENT = 4;

    // Identificador del cuerpo en el mundo fisico (Box2D)
    b2BodyId body{};
    b2ShapeId carShapeId{};
    CarParams params{};

    uint16_t model{0};

    // Vida del auto 0..100
    float health = 100;
    mutable int actual_crash = 0;
    mutable int one_destroy =
            DESTROY_EVENT;  // Solo se devolvera UNA VEZ POR auto el 4 (para indicar explocion).

    int level = 0;  // 0..1

    bool finished = false;
    bool god_mode = false;
    bool ghost = false;

    // para detectar frenadas fuertes
    mutable bool brake_sound_pending = false;  // si esta frenando
    mutable bool goal_sound_pending = false;   // si justo entro a la meta
    mutable bool was_braking = false;

    NpcState npc;
    // Diseño actual del auto
    CarDesignDef design_def{};
    void update_filter();

    static float lineal_interpoletion(float min, float max, float t);
    CarParams make_car_params_from_design(const CarDesignDef& def);

    // Helpers para apply_input (solo reordenan lógica, sin cambiarla)
    b2Vec2 get_forward_vector() const;
    float get_speed_along_forward(const b2Vec2& forward, const b2Vec2& vel) const;

    void apply_drive_forces(bool w, bool s, bool slow_zone, const b2Vec2& forward);
    void apply_direction(bool a, bool d, float speedAlong);
    void clamp_linear_speed();
    void damp_angular_velocity();

public:
    Car(b2WorldId worldId, float x, float y, float angle_rad, uint16_t model_id);
    void apply_input(bool w, bool s, bool a, bool d, bool slow_zone);

    bool is_on_slow_zone(const std::vector<Cell>& slow_cells, int height) const;

    b2Vec2 get_position() const;
    b2Rot get_rotation() const;
    uint16_t get_model() const;

    // Dependiendo el tipo de auto y el tipo de crash (0,1,2) se resta x # de vida
    void apply_damage(float speed);

    int get_and_consume_actual_crash() const;

    float get_health() const;
    bool is_destroyed() const;

    void set_user_data();

    int get_one_destroy() const;

    void set_layer(int z);

    int get_level() const { return level; }

    void mark_finished();
    bool is_finished() const;
    void set_ghost(bool ghost);

    void apply_upgrade(uint8_t upgrade);

    void kill();

    void set_god_mode(bool on);

    bool consume_goal_sound() const;

    bool consume_brake_sound() const;

    bool is_ghost() const { return ghost; }

    void force_set_transform(float x, float y, float angle_rad);
    void force_set_forward_speed(float speed);

    void make_npc(NpcDir initial_dir, float speed);
    bool is_npc() const;
    NpcState& npc_state();
    const NpcState& npc_state() const;

    ~Car() = default;
};

#endif  // CAR_H
