#ifndef MINI_MAP_RENDER_H
#define MINI_MAP_RENDER_H

#include <cstdint>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "ServerEvent.h"


const int MINIMAP_MARGIN = 15;
const int MINIMAP_WIDTH = 232;  // usamos IMAGE_WIDTH / 20
const int MINIMAP_HEIGHT = 234;


const int IMAGEN_WIDTH = 4640;
const int IMAGEN_HIGH = 4672;

const float MINIMAP_SCALE_X = static_cast<float>(MINIMAP_WIDTH) / IMAGEN_WIDTH;
const float MINIMAP_SCALE_Y = static_cast<float>(MINIMAP_HEIGHT) / IMAGEN_HIGH;

const int CHECKPOINT_SIZE = 8;
const int PLAYER_SIZE = 8;
const int NPC_SIZE = 4;

class MiniMapRender {
private:
    SDL2pp::Renderer& renderer;

    SDL2pp::Texture& background_texture;

    void render_checkpoints(const Player& main_player, int pos_x, int pos_y);

    void render_players(const std::vector<Player>& players, const Player& main_player, int pos_x,
                        int pos_y);

    void render_npc(const std::vector<NPC>& npcs, int pos_x, int pos_y);


public:
    void render_minimap(int window_widht, const std::vector<Player>& players,
                        const std::vector<NPC>& npcs, const Player& main_player);

    MiniMapRender(SDL2pp::Renderer& renderer, SDL2pp::Texture& background_texture);
};

#endif
