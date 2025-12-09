#include "car.h"

#include "../config.h"

#include "map_loader.h"


Car::Car(b2WorldId worldId, float x, float y, float angle_rad, uint16_t model_id): model(model_id) {

    const CarDesignDef& def = Config::instance().car_design(model);

    design_def = def;
    params = make_car_params_from_design(design_def);

    b2BodyDef defBody = b2DefaultBodyDef();
    defBody.type = b2_dynamicBody;
    defBody.position = {x, y};
    defBody.linearDamping = 1.5f;
    defBody.angularDamping = 2.0f;

    body = b2CreateBody(worldId, &defBody);

    b2Polygon box = b2MakeBox(params.length / 2.0f, params.width / 2.0f);
    b2ShapeDef shape = b2DefaultShapeDef();
    shape.density = params.density;
    shape.material.friction = params.friction;
    shape.enableSensorEvents = true;
    shape.enableHitEvents = true;

    shape.filter.categoryBits = CAR_L0;
    shape.filter.maskBits = CAR_L0 | WALL_L0 | NPC_L0 | SENSOR;

    carShapeId = b2CreatePolygonShape(body, &shape, &box);

    b2MassData md = b2Body_GetMassData(body);
    md.center = {-params.length * 0.20f, 0.0f};
    b2Body_SetMassData(body, md);

    b2Body_SetTransform(body, b2Vec2{x, y}, b2MakeRot(angle_rad));

    update_filter();
}

void Car::update_filter() {
    b2Filter f{};

    if (ghost) {
        if (level == 0) {
            f.categoryBits = npc.active ? NPC_L0 : CAR_L0;
        } else {
            f.categoryBits = npc.active ? NPC_L1 : CAR_L1;
        }
        f.maskBits = 0;
        b2Shape_SetFilter(carShapeId, f);
        return;
    }

    if (level == 0) {
        if (npc.active) {
            f.categoryBits = NPC_L0;
            f.maskBits = CAR_L0 | WALL_L0 | SENSOR;
        } else {
            f.categoryBits = CAR_L0;
            f.maskBits = CAR_L0 | NPC_L0 | WALL_L0 | SENSOR;
        }
    } else {
        if (npc.active) {
            f.categoryBits = NPC_L1;
            f.maskBits = CAR_L1 | WALL_L1 | SENSOR;
        } else {
            f.categoryBits = CAR_L1;
            f.maskBits = CAR_L1 | NPC_L1 | WALL_L1 | SENSOR;
        }
    }

    b2Shape_SetFilter(carShapeId, f);
}

// en un rango min - max, le pasamos un t que va entre 0 y 1 y nos devuelve ese porcentaje
float Car::lineal_interpoletion(float min, float max, float t) { return min + (max - min) * t; }

CarParams Car::make_car_params_from_design(const CarDesignDef& def) {

    CarParams p{};

    p.length = def.baseLength;
    p.width = def.baseWidth;

    float area = p.length * p.width;

    // Valores en % (en el yaml se ponen entre 0 y 100)
    float speed = def.stats.speed / 100.0f;
    float accel = def.stats.engine_force / 100.0f;
    float handling = def.stats.handling / 100.0f;
    float weight = def.stats.weight / 100.0f;
    float shield = def.stats.shield / 100.0f;

    const Config& cfg = Config::instance();

    // Mapeamos el peso entre 0 y 1 a la densidad (a mayor peso, mayor densidad)
    // Masa = densidad * area. A mayor densidad, el auto será mas pesado
    float minDensity = cfg.min_density();
    float maxDensity = cfg.max_density();
    p.density = lineal_interpoletion(minDensity, maxDensity, weight);

    // Misma idea con la velocidad
    float minMaxSpeed = cfg.min_max_speed();
    float maxMaxSpeed = cfg.max_max_speed();
    p.maxSpeed = lineal_interpoletion(minMaxSpeed, maxMaxSpeed, speed);


    // Aceleracion = Fuerza/masa -> A mayor fuerza, mayor aceleracion. Pero el peso del auto me
    // influye un monton -> Un auto pesado necesitara un motor con mucha aceleracion
    float minAccel = cfg.min_engine_force();
    float maxAccel = cfg.max_engine_force();
    float desiredAccel = lineal_interpoletion(minAccel, maxAccel, accel);

    // masa = densidad * area -> La fuera que necesitamos = m * a
    float massApprox = p.density * area;
    p.engineForce = massApprox * desiredAccel;

    // La comodidad del manejo viene dada por turnTorque (Que tan rapido el auto gira. Valores mas
    // altos lo hacen inmanejable) y la friccion (Como el auto se desliza sobre superficies. Un
    // valor mas alto, se desliza menos)
    float minTurnTorque = cfg.min_turn_torque();
    float maxTurnTorque = cfg.max_turn_torque();
    p.turnTorque = lineal_interpoletion(minTurnTorque, maxTurnTorque, handling);

    float minFriction = cfg.min_friction();
    float maxFriction = cfg.max_friction();
    p.friction = lineal_interpoletion(minFriction, maxFriction, handling);

    // A mayor shield (sin llegar nunca a 1 porque ahi curaria) mayor proteccion contra los golpes
    float minShield = cfg.min_shield();
    float maxShield = cfg.max_shield();
    p.shield = lineal_interpoletion(minShield, maxShield, shield);

    return p;
}

b2Vec2 Car::get_forward_vector() const {
    const float angle = b2Rot_GetAngle(b2Body_GetRotation(body));
    return {std::cos(angle), std::sin(angle)};
}

float Car::get_speed_along_forward(const b2Vec2& forward, const b2Vec2& vel) const {
    return b2Dot(vel, forward);
}

void Car::apply_drive_forces(bool w, bool s, bool slow_zone, const b2Vec2& forward) {

    const Config& cfg = Config::instance();

    float fwdForce = params.engineForce;
    if (slow_zone) {
        fwdForce *= cfg.slow_zone_factor();
    }
    const float reverseForce = fwdForce * cfg.reverse_factor();

    if (w && !s) {
        b2Body_ApplyForceToCenter(body, {forward.x * fwdForce, forward.y * fwdForce}, true);
    } else if (s && !w) {
        b2Body_ApplyForceToCenter(body, {-forward.x * reverseForce, -forward.y * reverseForce},
                                  true);
    } else {
        const b2Vec2 vel = b2Body_GetLinearVelocity(body);
        b2Body_ApplyForceToCenter(body, {-vel.x * DRAG_COEFF, -vel.y * DRAG_COEFF}, true);
    }
}

void Car::apply_direction(bool a, bool d, float speedAlong) {
    if (std::fabs(speedAlong) <= 0.2f) {
        return;
    }

    float turnDir = 0.0f;
    if (d)
        turnDir -= 1.0f;
    if (a)
        turnDir += 1.0f;

    if (turnDir == 0.0f) {
        return;
    }

    const float sign = (speedAlong >= 0.0f) ? 1.0f : -1.0f;
    b2Body_ApplyTorque(body, params.turnTorque * turnDir * sign, true);

    float omega = b2Body_GetAngularVelocity(body);
    if (std::fabs(omega) > MAX_ANGULAR_VEL) {
        b2Body_SetAngularVelocity(body, std::copysign(MAX_ANGULAR_VEL, omega));
    }
}

void Car::clamp_linear_speed() {
    b2Vec2 vel = b2Body_GetLinearVelocity(body);
    float speed = b2Length(vel);
    if (speed > params.maxSpeed) {
        float k = params.maxSpeed / speed;
        b2Body_SetLinearVelocity(body, {vel.x * k, vel.y * k});
    }
}

void Car::damp_angular_velocity() {
    float omega = b2Body_GetAngularVelocity(body);
    b2Body_ApplyTorque(body, -omega * EXTRA_ANGULAR_DAMPING, true);
}


void Car::apply_input(bool w, bool s, bool a, bool d, bool slow_zone) {

    // Si el auto se destruyo o ya termino, no se puede mover mas
    if (is_destroyed() || finished) {
        return;
    }

    const b2Vec2 forward = get_forward_vector();
    b2Vec2 vel = b2Body_GetLinearVelocity(body);
    const float speedAlong = get_speed_along_forward(forward, vel);
    const float forwardSpeed = std::max(speedAlong, 0.0f);

    const bool braking_now = (s && forwardSpeed > MIN_FORWARD_SPEED_FOR_BRAKE);

    // Si AHORA esta frenando y ANTES no, disparamos sonido
    if (braking_now && !was_braking) {
        brake_sound_pending = true;
    }

    apply_drive_forces(w, s, slow_zone, forward);
    apply_direction(a, d, speedAlong);
    clamp_linear_speed();
    damp_angular_velocity();

    was_braking = braking_now;
}

b2Vec2 Car::get_position() const { return b2Body_GetPosition(body); }

b2Rot Car::get_rotation() const { return b2Body_GetRotation(body); }

uint16_t Car::get_model() const { return model; }

void Car::set_user_data() { b2Body_SetUserData(body, this); }

// Dado el vector de celdas "lentas" detecta si el auto esta en alguna
bool Car::is_on_slow_zone(const std::vector<Cell>& slow_cells, int height) const {
    b2Vec2 pos = get_position();
    int col = static_cast<int>(pos.x);
    int row = height - 1 - static_cast<int>(pos.y);

    auto it = std::find_if(slow_cells.begin(), slow_cells.end(),
                           [col, row](const Cell& c) { return c.col == col && c.row == row; });

    return it != slow_cells.end();
}

float Car::get_health() const { return health; }

bool Car::is_destroyed() const { return health <= 0.0f; }

void Car::apply_damage(float speed) {

    if (npc.active) {
        kill();
    }

    if (god_mode) {
        return;
    }

    const Config& cfg = Config::instance();

    float v = speed;
    if (v < CRASH_SPEED_LOW) {
        health -= cfg.damage_low() * (1.0f - params.shield);
        actual_crash = 1;
    } else if (v < CRASH_SPEED_MED) {
        health -= cfg.damage_med() * (1.0f - params.shield);
        actual_crash = 2;
    } else {
        health -= cfg.damage_high() * (1.0f - params.shield);
        actual_crash = 3;
    }
}

// Para que la snapshot muestre que hubo choque, pero no se sature de choques
int Car::get_and_consume_actual_crash() const {
    int ac = actual_crash;
    actual_crash = 0;
    return ac;
}

int Car::get_one_destroy() const {
    int aux = 0;
    if (one_destroy == DESTROY_EVENT) {
        aux = one_destroy;
        one_destroy = 0;
    }
    return aux;
}

void Car::set_layer(int z) {

    if (ghost) {
        return;
    }

    level = z;
    update_filter();
}

void Car::mark_finished() {
    if (!finished) {
        finished = true;
        goal_sound_pending = true;
    }
}

bool Car::is_finished() const { return finished; }

void Car::set_ghost(bool ghost) {
    this->ghost = ghost;
    update_filter();
}

void Car::apply_upgrade(uint8_t upgrade) {
    UpgradesOfACar up = static_cast<UpgradesOfACar>(upgrade);
    switch (up) {
        case UpgradesOfACar::Vel1:
            params.maxSpeed *= 1.08f;
            params.engineForce *= 1.05f;
            break;
        case UpgradesOfACar::Vel2:
            params.maxSpeed *= 1.16f;
            params.engineForce *= 1.10f;
            break;
        case UpgradesOfACar::Vel3:
            params.maxSpeed *= 1.25f;
            params.engineForce *= 1.15f;
            break;

        case UpgradesOfACar::Shield1:
            params.shield = std::min(params.shield + 0.10f, 0.9f);
            break;
        case UpgradesOfACar::Shield2:
            params.shield = std::min(params.shield + 0.20f, 0.9f);
            break;
        case UpgradesOfACar::Shield3:
            params.shield = std::min(params.shield + 0.30f, 0.9f);
            break;

        case UpgradesOfACar::Handle1:
            params.turnTorque *= 1.05f;
            params.friction *= 1.05f;
            break;
        case UpgradesOfACar::Handle2:
            params.turnTorque *= 1.10f;
            params.friction *= 1.10f;
            break;
        case UpgradesOfACar::Handle3:
            params.turnTorque *= 1.15f;
            params.friction *= 1.15f;
            break;

        default:
            break;
    }
}

void Car::kill() {
    health = 0.0f;
    // Los npcs nunca los ponemos ghost cuando se mueren, asi es mas divertido.
    if (!npc.active) {
        set_ghost(true);
    }
}

void Car::set_god_mode(bool on) {
    god_mode = on;
    health = 100.0f;
}

bool Car::consume_brake_sound() const {
    bool res = brake_sound_pending;
    brake_sound_pending = false;
    return res;
}

bool Car::consume_goal_sound() const {
    bool res = goal_sound_pending;
    goal_sound_pending = false;
    return res;
}

void Car::force_set_transform(float x, float y, float angle_rad) {
    // Solo podemos manipular a los npcs
    if (npc.active == false) {
        return;
    }
    b2Body_SetTransform(body, b2Vec2{x, y}, b2MakeRot(angle_rad));
}

void Car::force_set_forward_speed(float speed) {
    // Solo podemos manipular a los npcs
    if (npc.active == false || is_destroyed()) {
        return;
    }
    const float angle = b2Rot_GetAngle(b2Body_GetRotation(body));
    b2Vec2 vel{std::cos(angle) * speed, std::sin(angle) * speed};
    b2Body_SetLinearVelocity(body, vel);
}

void Car::make_npc(NpcDir initial_dir, float speed) {
    npc.active = true;
    npc.dir = initial_dir;
    npc.speed = speed;
    npc.steps_since_last_turn = 1000;
    update_filter();
}

bool Car::is_npc() const { return npc.active; }
NpcState& Car::npc_state() { return npc; }
const NpcState& Car::npc_state() const { return npc; }
