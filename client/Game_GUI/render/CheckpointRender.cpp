#include "CheckpointRender.h"

#include "../../../common/resource_paths.h"


CheckpointRender::CheckpointRender(SDL2pp::Renderer& renderer, SDL2pp::Window& window):
        renderer(renderer),
        window(window),
        start_line_img(ResourcePaths::assets() + "/client_img_render/start_line.png"),
        start_line_texture(renderer, start_line_img) {}


void CheckpointRender::render_normal_checkpoint(const Player& player, Coords window_left_corner) {
    int window_width, window_high;
    SDL_GetWindowSize(window.Get(), &window_width, &window_high);

    SDL_SetRenderDrawBlendMode(renderer.Get(), SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(renderer.Get(), 0, 255, 0, 128);  // Verde

    for (const auto& checkpoint: player.next_checkpoint) {
        int position_x = checkpoint.coord_x - window_left_corner.coord_x;
        int position_y = checkpoint.coord_y - window_left_corner.coord_y;

        if (position_x >= 0 && position_x < window_width && position_y >= 0 &&
            position_y < window_high) {
            SDL_Rect point = {position_x - SIZE_TILE / 2, position_y - SIZE_TILE / 2, SIZE_TILE,
                              SIZE_TILE};
            SDL_RenderFillRect(renderer.Get(), &point);
        }
    }

    if (!player.is_secondary_check) {
        return;
    }

    SDL_SetRenderDrawColor(renderer.Get(), 0, 70, 0, 128);


    for (const auto& checkpoint: player.secondary_checkpoint) {
        int position_x = checkpoint.coord_x - window_left_corner.coord_x;
        int position_y = checkpoint.coord_y - window_left_corner.coord_y;

        if (position_x >= 0 && position_x < window_width && position_y >= 0 &&
            position_y < window_high) {
            SDL_Rect point = {position_x - SIZE_TILE / 2, position_y - SIZE_TILE / 2, SIZE_TILE,
                              SIZE_TILE};
            SDL_RenderFillRect(renderer.Get(), &point);
        }
    }
}


void CheckpointRender::render_finishline(const Player& player, Coords window_left_corner) {

    int window_width, window_high;
    SDL_GetWindowSize(window.Get(), &window_width, &window_high);

    SDL_SetRenderDrawBlendMode(renderer.Get(), SDL_BLENDMODE_BLEND);

    SDL_SetRenderDrawColor(renderer.Get(), 255, 255, 255, 255);  // Blanco

    int i = 0;  // para alternar cuadrado entre negro y blanco

    for (const auto& checkpoint: player.next_checkpoint) {
        int position_x = checkpoint.coord_x - window_left_corner.coord_x;
        int position_y = checkpoint.coord_y - window_left_corner.coord_y;

        if (position_x >= 0 && position_x < window_width && position_y >= 0 &&
            position_y < window_high) {
            SDL_Rect point = {position_x - SIZE_TILE / 2, position_y - SIZE_TILE / 2, SIZE_TILE,
                              SIZE_TILE};
            SDL_RenderFillRect(renderer.Get(), &point);

            if (i % 2 == 0) {
                SDL_SetRenderDrawColor(renderer.Get(), 0, 0, 0, 255);  // Negro
            } else {
                SDL_SetRenderDrawColor(renderer.Get(), 255, 255, 255, 255);  // Blanco
            }
            i++;
        }
    }
}

void CheckpointRender::render_start_line(Coords window_left_corner, StartLine start_line) {
    SDL_Rect src_rect{};

    int dest_w = STARTLINE_WIDTH;
    int dest_h = STARTLINE_HEIGHT;

    switch (start_line.direction) {
        case Direction::RIGHT:
            src_rect = {11, 26, STARTLINE_WIDTH, STARTLINE_HEIGHT};
            break;
        case Direction::LEFT:
            src_rect = {11, 110, STARTLINE_WIDTH, STARTLINE_HEIGHT};
            break;
        case Direction::UP:
            src_rect = {382, 9, STARTLINE_HEIGHT, STARTLINE_WIDTH};
            dest_w = STARTLINE_HEIGHT;  // Roto al estar vertical
            dest_h = STARTLINE_WIDTH;
            break;
        case Direction::DOWN:
            src_rect = {289, 9, STARTLINE_HEIGHT, STARTLINE_WIDTH};
            dest_w = STARTLINE_HEIGHT;
            dest_h = STARTLINE_WIDTH;
            break;
        default:
            return;
    }

    // Calculamos dónde se dibuja en pantalla
    int window_width, window_height;
    SDL_GetWindowSize(window.Get(), &window_width, &window_height);

    int dest_x = start_line.top_left.coord_x - window_left_corner.coord_x;
    int dest_y = start_line.top_left.coord_y - window_left_corner.coord_y;

    SDL_Rect dst_rect = {dest_x, dest_y, dest_w, dest_h};

    SDL_RenderCopy(renderer.Get(), start_line_texture.Get(), &src_rect, &dst_rect);
}


void CheckpointRender::render_checkpoint(const Player& player, Coords window_left_corner) {
    if (!player.is_checkpoint_finishline) {
        render_normal_checkpoint(player, window_left_corner);
    } else {
        render_finishline(player, window_left_corner);
    }
}
