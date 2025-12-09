#include "CarRender.h"

#include "../../../common/resource_paths.h"


CarRender::CarRender(SDL2pp::Renderer& renderer):
        renderer(renderer),
        vehicle_img(ResourcePaths::assets() +
                    "/need4speed_assets/Mobile - Grand Theft Auto 4 - Miscellaneous "
                    "- Cars.png"),
        vehicle(renderer, vehicle_img.SetColorKey(
                                  true, SDL_MapRGB(vehicle_img.Get()->format, 163, 163, 13))) {
    vehicle.SetBlendMode(SDL_BLENDMODE_BLEND);  // Para sacar el fondo verde
}


void CarRender::render_car_generic(int src_x, int src_y, int tile, Coords car_cords,
                                   Coords window_left_corner) {

    SDL2pp::Rect src_rect(src_x, src_y, tile, tile);

    int window_x = static_cast<int>(car_cords.coord_x) -
                   static_cast<int>(window_left_corner.coord_x) - tile / 2;
    int window_y = static_cast<int>(car_cords.coord_y) -
                   static_cast<int>(window_left_corner.coord_y) - tile / 2;

    SDL2pp::Rect dst_rect(window_x, window_y, tile, tile);

    renderer.Copy(vehicle, src_rect, dst_rect);
}


void CarRender::render_smallcar(uint32_t rotation, Coords car_cords, Coords window_left_corner) {

    int tile_car = TILE_SMALLCAR;

    int src_x = (rotation % CAR_COLUMNS) * tile_car;
    int src_y = (rotation / CAR_COLUMNS) * tile_car;

    render_car_generic(src_x, src_y, TILE_SMALLCAR, car_cords, window_left_corner);
}


void CarRender::render_mediumcar(uint32_t model_id, uint32_t rotation, Coords car_cords,
                                 Coords window_left_corner) {
    uint32_t tile_car = TILE_MEDIUMCAR;

    uint32_t src_x = (rotation % CAR_COLUMNS) * tile_car;
    uint32_t src_y = (rotation / CAR_COLUMNS) * tile_car + 64 + 80 * (model_id - 1);

    render_car_generic(src_x, src_y, TILE_MEDIUMCAR, car_cords, window_left_corner);
}


void CarRender::render_bigcar(uint32_t rotation, Coords car_cords, Coords window_left_corner) {
    int tile_car = TILE_BIGCAR;

    int src_x = (rotation % CAR_COLUMNS) * tile_car;
    int src_y = (rotation / CAR_COLUMNS) * tile_car + 464;

    render_car_generic(src_x, src_y, TILE_BIGCAR, car_cords, window_left_corner);
}


void CarRender::render_npc(const NPC& player, Coords window_left_corner, int window_width,
                           int window_high) {
    const auto& pos = player.pos;
    int coord_x = static_cast<int>(pos.coord_x - window_left_corner.coord_x);
    int coord_y = static_cast<int>(pos.coord_y - window_left_corner.coord_y);

    // Verifica que el jugador este dentro de la ventana
    if (coord_x > window_width || coord_y > window_high || coord_x < 0 || coord_y < 0)
        return;

    uint32_t model_id = player.model;
    uint32_t rotation = angle_to_asset(static_cast<float>(player.rotation));


    if (model_id == 0) {
        render_smallcar(rotation, pos, window_left_corner);
    } else if (model_id < 6) {
        render_mediumcar(model_id, rotation, pos, window_left_corner);
    } else if (model_id == 6) {
        render_bigcar(rotation, pos, window_left_corner);
    }
}


void CarRender::render_car(const Player& player, Coords window_left_corner, int window_width,
                           int window_high) {
    const auto& pos = player.player_position;
    int coord_x = static_cast<int>(pos.coord_x - window_left_corner.coord_x);
    int coord_y = static_cast<int>(pos.coord_y - window_left_corner.coord_y);

    // Verifica que el jugador este dentro de la ventana
    if (coord_x > window_width || coord_y > window_high || coord_x < 0 || coord_y < 0)
        return;

    uint32_t model_id = player.car_model;
    uint32_t rotation = angle_to_asset(static_cast<float>(player.rotation));

    if (player.is_car_ghost) {
        vehicle.SetAlphaMod(128);  // 50% transparente
    } else {
        vehicle.SetAlphaMod(255);  //  normal
    }

    if (model_id == 0) {
        render_smallcar(rotation, pos, window_left_corner);
    } else if (model_id < 6) {
        render_mediumcar(model_id, rotation, pos, window_left_corner);
    } else if (model_id == 6) {
        render_bigcar(rotation, pos, window_left_corner);
    }

    vehicle.SetAlphaMod(255);
}


int CarRender::angle_to_asset(float angle) {
    // Convertir a convención de pantalla (Y hacia abajo, horario)
    float screen_deg = 360.0f - angle;
    if (screen_deg >= 360.0f)
        screen_deg -= 360.0f;

    //  Cuantizar a 16 frames con redondeo al más cercano
    const float sector = 360.0f / 16.0;
    int idx = static_cast<int>((screen_deg + sector / 2.0f) / sector) & 15;

    return idx;  // 0..15 (0=derecha, 4=abajo, 8=izq, 12=arriba)
}
