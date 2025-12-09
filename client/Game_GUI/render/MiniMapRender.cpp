#include "MiniMapRender.h"

#include <vector>

MiniMapRender::MiniMapRender(SDL2pp::Renderer& renderer, SDL2pp::Texture& background_texture):
        renderer(renderer), background_texture(background_texture) {}

void MiniMapRender::render_minimap(int window_widht, const std::vector<Player>& players,
                                   const std::vector<NPC>& npcs, const Player& main_player) {

    SDL_Rect minimap_position = {window_widht - MINIMAP_WIDTH - MINIMAP_MARGIN, MINIMAP_MARGIN,
                                 MINIMAP_WIDTH, MINIMAP_HEIGHT};

    SDL2pp::Rect minimap_window(0, 0, IMAGEN_WIDTH, IMAGEN_HIGH);

    renderer.Copy(background_texture, minimap_window, minimap_position);


    render_checkpoints(main_player, minimap_position.x, minimap_position.y);

    render_players(players, main_player, minimap_position.x, minimap_position.y);

    // Comentar para sacar npcs del mini-mapa
    render_npc(npcs, minimap_position.x, minimap_position.y);
}


void MiniMapRender::render_checkpoints(const Player& main_player, int pos_x, int pos_y) {
    SDL_SetRenderDrawColor(renderer.Get(), 0, 255, 0, 180);  // checkpoints verde
    for (const auto& checkpoint: main_player.next_checkpoint) {
        int checkpoint_x = pos_x + static_cast<int>(checkpoint.coord_x * MINIMAP_SCALE_X);
        int checkpoint_y = pos_y + static_cast<int>(checkpoint.coord_y * MINIMAP_SCALE_Y);
        SDL_Rect point = {checkpoint_x - CHECKPOINT_SIZE / 2, checkpoint_y - CHECKPOINT_SIZE / 2,
                          CHECKPOINT_SIZE, CHECKPOINT_SIZE};
        SDL_RenderFillRect(renderer.Get(), &point);
    }
}

void MiniMapRender::render_npc(const std::vector<NPC>& npcs, int pos_x, int pos_y) {
    for (const NPC& npc: npcs) {
        SDL_SetRenderDrawColor(renderer.Get(), 255, 105, 180, 255);  // competidores color rosa

        int coord_x = pos_x + static_cast<int>(npc.pos.coord_x * MINIMAP_SCALE_X);
        int coord_y = pos_y + static_cast<int>(npc.pos.coord_y * MINIMAP_SCALE_Y);
        SDL_Rect player_rect = {coord_x - NPC_SIZE / 2, coord_y - NPC_SIZE / 2, NPC_SIZE, NPC_SIZE};
        SDL_RenderFillRect(renderer.Get(), &player_rect);
    }
}


void MiniMapRender::render_players(const std::vector<Player>& players, const Player& main_player,
                                   int pos_x, int pos_y) {

    for (const Player& player: players) {
        if (player.user_id == main_player.user_id) {
            SDL_SetRenderDrawColor(renderer.Get(), 0, 0, 255, 255);  // main player color azul
        } else {
            SDL_SetRenderDrawColor(renderer.Get(), 255, 0, 0, 255);  // competidores color rojo
        }

        int coord_x = pos_x + static_cast<int>(player.player_position.coord_x * MINIMAP_SCALE_X);
        int coord_y = pos_y + static_cast<int>(player.player_position.coord_y * MINIMAP_SCALE_Y);
        SDL_Rect player_rect = {coord_x - PLAYER_SIZE / 2, coord_y - PLAYER_SIZE / 2, PLAYER_SIZE,
                                PLAYER_SIZE};
        SDL_RenderFillRect(renderer.Get(), &player_rect);
    }
}
