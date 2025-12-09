#ifndef CAR_RENDER_H
#define CAR_RENDER_H

#include <cstdint>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "ServerEvent.h"


const int CAR_COLUMNS = 8;

const int TILE_SMALLCAR = 32;
const int TILE_MEDIUMCAR = 40;
const int TILE_BIGCAR = 48;

class CarRender {

    void render_smallcar(uint32_t rotation, Coords car_cords, Coords window_left_corner);

    void render_mediumcar(uint32_t model_id, uint32_t rotation, Coords car_cords,
                          Coords window_left_corner);

    void render_bigcar(uint32_t rotation, Coords car_cords, Coords window_left_corner);


    void render_car_generic(int src_x, int src_y, int tile, Coords car_cords,
                            Coords window_left_corner);

    int angle_to_asset(float angle);

    SDL2pp::Renderer& renderer;

    SDL2pp::Surface vehicle_img;

    SDL2pp::Texture vehicle;

public:
    explicit CarRender(SDL2pp::Renderer& renderer);

    void render_npc(const NPC& player, Coords window_left_corner, int window_width,
                    int window_high);

    void render_car(const Player& player, Coords window_left_corner, int window_width,
                    int window_high);
};

#endif
