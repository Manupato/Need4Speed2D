#include "HintArrowRender.h"

#include "../../../common/resource_paths.h"

HintArrowRender::HintArrowRender(SDL2pp::Renderer& renderer):
        renderer(renderer),
        arrow_surf(ResourcePaths::assets() + "/client_img_render/arrows_hints.png"),
        arrow_text(renderer, arrow_surf),
        srcSDL(SRC_X_ARROW, SRC_Y_ARROW, SCREEN_WIDHT_ARROW, SCREEN_HEIGHT_ARROW) {}


void HintArrowRender::render_arrow(const Player& main_player, const SDL2pp::Window& window) {
    if (main_player.next_checkpoint.empty()) {
        return;
    }
    int window_width, window_high;  // Agarra tamaño ventana
    SDL_GetWindowSize(window.Get(), &window_width, &window_high);

    Coords checkpoint_pos = main_player.next_checkpoint[main_player.next_checkpoint.size() / 2];
    Coords player_pos = main_player.player_position;


    int x = checkpoint_pos.coord_x - player_pos.coord_x;
    int y = checkpoint_pos.coord_y - player_pos.coord_y;

    int angle = atan2(y, x) * 180 / M_PI;
    angle += 90;  // esto porque usamos imagen con flecha ya en 90°

    if (angle < 0)
        angle += 360.0;
    else if (angle >= 360)
        angle -= 360;

    SDL2pp::Rect dstSDL(20, window_high - ARROW_SIZE - 20, ARROW_SIZE, ARROW_SIZE);

    renderer.Copy(arrow_text, srcSDL, dstSDL, angle, SDL2pp::NullOpt, SDL_FLIP_NONE);
}
