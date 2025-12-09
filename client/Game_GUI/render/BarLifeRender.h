#ifndef BAR_LIFE_RENDER_H
#define BAR_LIFE_RENDER_H

#include <cstdint>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "ServerEvent.h"

const int SCREEN_BL_X = 10;
const int SCREEN_BL_Y = 10;

const int SCREEN_BL_WIDTH = 220;
const int SCREEN_BL_HEIGHT = 90;

// src_empty_bar(13, 6, 60, 16),

const int SRC_BL_X = 13;
const int SRC_BL_Y = 6;

const int SRC_BL_WIDTH = 60;
const int SRC_BL_HEIGHT = 16;

class BarLifeRender {
private:
    SDL2pp::Renderer& renderer;

    SDL2pp::Surface barlife_img;

    SDL2pp::Texture barlife_texture;

    SDL2pp::Rect src_empty_bar;

    SDL2pp::Rect dst_empty_bar;

public:
    void render_barlife(uint16_t car_life);

    explicit BarLifeRender(SDL2pp::Renderer& renderer);
};

#endif
