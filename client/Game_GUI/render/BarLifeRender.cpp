#include "BarLifeRender.h"

#include <algorithm>

#include "../../../common/resource_paths.h"

BarLifeRender::BarLifeRender(SDL2pp::Renderer& renderer):
        renderer(renderer),
        barlife_img(ResourcePaths::assets() + "/client_img_render/barlife_cleanxcf.png"),
        barlife_texture(renderer, barlife_img),
        src_empty_bar(SRC_BL_X, SRC_BL_Y, SRC_BL_WIDTH, SRC_BL_HEIGHT),
        dst_empty_bar(SCREEN_BL_X, SCREEN_BL_Y, SCREEN_BL_WIDTH, SCREEN_BL_HEIGHT) {}


void BarLifeRender::render_barlife(uint16_t car_life) {

    float life_percent = std::max(0.0f, std::min(1.0f, car_life / 100.0f));

    int fill_x = SCREEN_BL_X + (24 - 13) * (SCREEN_BL_WIDTH / 60.0);
    int fill_y = SCREEN_BL_Y + (11 - 6) * (SCREEN_BL_HEIGHT / 16.0);

    int fill_w = life_percent * (42 * (SCREEN_BL_WIDTH / 60.0));
    int fill_h = 8 * (SCREEN_BL_HEIGHT / 16.0);

    if (car_life > 60) {
        SDL_SetRenderDrawColor(renderer.Get(), 0, 200, 0, 255);  // Verde
    } else if (car_life > 30) {
        SDL_SetRenderDrawColor(renderer.Get(), 255, 165, 0, 255);  // Verde
    } else {
        SDL_SetRenderDrawColor(renderer.Get(), 255, 0, 0, 255);  // Rojo
    }

    renderer.FillRect(SDL2pp::Rect(fill_x, fill_y, fill_w, fill_h));

    renderer.Copy(barlife_texture, src_empty_bar, dst_empty_bar);
}
