#ifndef CHECKPOINT_RENDER_H
#define CHECKPOINT_RENDER_H

#include <cstdint>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "ServerEvent.h"


const int SIZE_TILE = 16;

const int STARTLINE_WIDTH = 240;
const int STARTLINE_HEIGHT = 64;

class CheckpointRender {
private:
    SDL2pp::Renderer& renderer;

    SDL2pp::Window& window;

    SDL2pp::Surface start_line_img;

    SDL2pp::Texture start_line_texture;

    void render_normal_checkpoint(const Player& player, Coords window_left_corner);

    void render_finishline(const Player& player, Coords window_left_corner);


public:
    void render_checkpoint(const Player& player, Coords window_left_corner);

    void render_start_line(Coords window_left_corner, StartLine strart_line);

    CheckpointRender(SDL2pp::Renderer& renderer, SDL2pp::Window& window);
};

#endif
