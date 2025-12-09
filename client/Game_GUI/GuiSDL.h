#ifndef GUISDL_H
#define GUISDL_H

#include <iostream>
#include <optional>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "render/BarLifeRender.h"
#include "render/CarRender.h"
#include "render/CarRenderAnimation.h"
#include "render/CheckpointRender.h"
#include "render/HintArrowRender.h"
#include "render/MiniMapRender.h"
#include "render/MuteSimbolRender.h"
#include "render/RenderCarUpgrades.h"
#include "render/RenderPositions.h"
#include "render/TimeRender.h"

#include "ServerEvent.h"


const int IMAGE_WIDTH = 4640;
const int IMAGE_HIGH = 4672;

const int WINDOW_INITIAL_WIDTH = 1280;
const int WINDOW_INITIAL_HIGH = 720;


class GuiSDL {
private:
    SDL2pp::SDL sdl;
    SDL2pp::Window window;
    SDL2pp::Renderer renderer;

    SDL2pp::Texture background_texture;
    SDL2pp::Texture background_texture_lv1;

    CarRender render_car;
    BarLifeRender render_barlife;
    CheckpointRender render_checkpoint;
    MiniMapRender render_minimap;
    CarRenderAnimation render_caranimation;
    TimeRender time_render;
    HintArrowRender arrow_render;
    MuteSimbolRender mute_render;

    RenderPositions render_positions;
    RenderCarUpgrades render_carupgrades;

    StartLine start_line;


    void render_top_layer(const Snapshot& snapshot, const Player& main_player,
                          Coords window_left_corner);

    void render_bottom_layer(const Snapshot& snapshot, Coords window_left_corner);

public:
    GuiSDL();

    bool get_event(SDL_Event& out);


    void render_gameloop(const Snapshot& snapshot, const Player& main_player, const bool is_muted);

    void render_screen_position(const RaceResults& race_results);

    void render_screen_upgrades(bool clear_upgrades);

    void handle_upgrades(const SDL_Event& event, CarUpgrades& current_upgrade);

    void set_background(Map map_selected);

    void set_start_line(StartLine start_line) { this->start_line = start_line; }

    void set_start_time(uint32_t start_time);

    ~GuiSDL();
};

#endif
