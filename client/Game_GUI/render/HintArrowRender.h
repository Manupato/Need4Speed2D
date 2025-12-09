#ifndef HINT_ARROW_RENDER_H
#define HINT_ARROW_RENDER_H

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "ServerEvent.h"


const int ARROW_SIZE = 96;
const int SRC_X_ARROW = 34;
const int SRC_Y_ARROW = 33;

const int SCREEN_WIDHT_ARROW = 210;
const int SCREEN_HEIGHT_ARROW = 230;


class HintArrowRender {
private:
    SDL2pp::Renderer& renderer;

    SDL2pp::Surface arrow_surf;

    SDL2pp::Texture arrow_text;

    SDL2pp::Rect srcSDL;

public:
    explicit HintArrowRender(SDL2pp::Renderer& renderer);

    void render_arrow(const Player& main_player, const SDL2pp::Window& window);
};


#endif
