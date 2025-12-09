#ifndef WORLD_STATE_H
#define WORLD_STATE_H

#include <list>
#include <map>
#include <vector>

#include "car.h"
#include "physic_world.h"
#include "race_progress.h"

struct teclas_presionadas {
    bool w = false, a = false, s = false, d = false;
};

class WorldState {
private:
    // Cada cuántos steps como mínimo dejamos doblar de nuevo a un NPC (para que no quede girando)
    static constexpr int MIN_STEPS_BETWEEN_TURNS = 200;

    // Mapa de client_id a su auto
    std::map<int, Car> cars;

    // Mapa de cliente a sus ultimos comandos recibidos
    std::map<int, teclas_presionadas> player_movements;

    // Mapea id cliente con su progreso en carrera (por ahora, solo el sig. checkpoint)
    std::map<int, RaceProgress> race_progress;

    PhysicWorld& pw;

    // npcs
    std::list<Car> npc_cars;

    // Helpers internos
    teclas_presionadas& inputs_for(int client_id);
    RaceProgress& progress_for(int client_id);

    static float angle_for_npc_dir(NpcDir dir);
    static NpcDir left_of(NpcDir dir);
    static NpcDir right_of(NpcDir dir);
    static NpcDir opposite_of(NpcDir dir);
    bool can_go(const Car& car, NpcDir dir);

public:
    explicit WorldState(PhysicWorld& pw);

    bool client_have_car(const int client_id) const;

    // Actualiza teclas del jugador
    void change_w(bool new_state, int client_id);
    void change_a(bool new_state, int client_id);
    void change_s(bool new_state, int client_id);
    void change_d(bool new_state, int client_id);

    std::size_t number_of_players() const;

    // Agrega un nuevo jugador y/o auto al juego!
    void add_new_car(Spawn&& spawn, uint16_t new_car_model, int client_id, b2WorldId worldId);

    // Aplica los inputs de los jugadores a sus respectivos autos
    void apply_player_inputs();

    RaceProgress& get_progress_of(int ownerClientId) { return race_progress[ownerClientId]; }

    void set_race_finish(int id_player, double time_finish);

    void win(int client_id, double race_with_countdown);
    void lose(int client_id);
    void undestroyable(int client_id);
    void ghost(int client_id);

    void spawn_npc(Spawn&& spawn, uint16_t model, NpcDir dir, float speed);
    void update_npcs();
    const std::list<Car>& get_npc_cars() const { return npc_cars; }

    int get_owner_id(const Car* car) const;

    std::map<int, Car>& get_cars() { return cars; }
    const std::map<int, Car>& get_cars() const { return cars; }

    std::map<int, RaceProgress>& get_race_progress() { return race_progress; }
    const std::map<int, RaceProgress>& get_race_progress() const { return race_progress; }


    ~WorldState() = default;
    WorldState(const WorldState& other) = delete;
    WorldState& operator=(const WorldState& other) = delete;
};

#endif  // WORLD_STATE_H
