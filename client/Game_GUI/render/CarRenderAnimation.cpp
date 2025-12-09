#include "CarRenderAnimation.h"

#include "../../../common/resource_paths.h"

CarRenderAnimation::CarRenderAnimation(SDL2pp::Renderer& renderer):
        renderer(renderer),
        animation_surf(ResourcePaths::assets() + "/client_img_render/animations.png"),
        animation_text(renderer, animation_surf) {}

void CarRenderAnimation::render_animation(const Player& player, Coords window_left_corner,
                                          int window_width, int window_high) {

    Animation& anim = car_animations.try_emplace(player.user_id, Animation{}).first->second;

    if (player.car_animation > NONE_ANIMATION) {


        anim.activated = true;
        anim.current_frame = 0;
        anim.delay_counter = 0;

        switch (player.car_animation) {
            case 1:
                anim.total_frames = SMALL_ANIMATION;
                break;
            case 2:
                anim.total_frames = MEDIUM_ANIMATION;
                break;
            case 3:
                anim.total_frames = BIG_ANIMATION;
                break;
            case 4:
                anim.total_frames = EXPLOSION_ANIMATION;
                break;
            default:
                anim.total_frames = NONE_ANIMATION;
                break;
        }
    }

    if (anim.activated) {
        render_frame(player, anim.current_frame, window_left_corner, window_width, window_high);
        anim.delay_counter++;
        if (anim.delay_counter >= DELAY) {
            anim.delay_counter = 0;
            anim.current_frame++;
        }

        if (anim.current_frame >= anim.total_frames) {
            anim.activated = false;
            anim.current_frame = 0;
        }
    }
}

void CarRenderAnimation::clear_animations() {
    for (auto& car: car_animations) {
        car.second.activated = false;
        car.second.current_frame = 0;
    }
}


void CarRenderAnimation::render_frame(const Player& player, int animation_number,
                                      Coords window_left_corner, int window_width,
                                      int window_high) {

    if (animation_number < 0 || animation_number >= TOTAL_FRAMES) {
        return;
    }

    int row = animation_number / FRAMES_COLUMNS;
    int col = animation_number % FRAMES_COLUMNS;

    SDL2pp::Rect src_rect(col * FRAMES_AREA, row * FRAMES_AREA, FRAMES_AREA, FRAMES_AREA);

    int coord_x = static_cast<int>(player.player_position.coord_x) - window_left_corner.coord_x;
    int coord_y = static_cast<int>(player.player_position.coord_y) - window_left_corner.coord_y;


    if (coord_x > window_width || coord_y > window_high || coord_x < 0 || coord_y < 0) {
        return;
    }

    int scaled = FRAMES_AREA / 3;  // cambiar el "/ 3" para achicar o agrandar animacion
    SDL2pp::Rect dst_rect(coord_x - scaled / 2, coord_y - scaled / 2, scaled, scaled);

    renderer.Copy(animation_text, src_rect, dst_rect);
}
