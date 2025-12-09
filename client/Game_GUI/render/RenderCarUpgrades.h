#ifndef RENDER_CAR_UPGRADES_H
#define RENDER_CAR_UPGRADES_H

#include <cstdint>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "Button.h"
#include "ServerEvent.h"


const int BTN_WIDTH = 200;
const int BTN_HEIGHT = 50;

const int MARGIN = 20;

class RenderCarUpgrades {
private:
    SDL2pp::Renderer& renderer;

    SDL2pp::Surface background_label_img;

    SDL2pp::Texture background_label;

    SDL2pp::Surface background_img;

    SDL2pp::Texture background;

    std::vector<Button> upgrade_buttons;

    void create_buttons(int table_x, int table_y);

    bool needes_resize;

public:
    explicit RenderCarUpgrades(SDL2pp::Renderer& renderer);

    void render_upgrades(SDL2pp::Window& window, bool clear_upgrades);

    void handle_event(const SDL_Event& e, CarUpgrades& current_upgrade);
};

#endif
