#ifndef MUTE_SIMBOL_RENDER_H
#define MUTE_SIMBOL_RENDER_H

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "ServerEvent.h"

const int MUTE_SIZE = 48;

const int SRC_MUTE_POS = 0;

class MuteSimbolRender {
private:
    SDL2pp::Renderer& renderer;

    SDL2pp::Surface mute_img;

    SDL2pp::Texture mute_text;

    SDL2pp::Rect src_rect;

public:
    void render_mute_simbol(int window_width, int window_high);

    explicit MuteSimbolRender(SDL2pp::Renderer& renderer);
};

#endif
