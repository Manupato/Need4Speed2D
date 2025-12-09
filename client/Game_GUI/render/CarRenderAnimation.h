#ifndef CAR_RENDER_ANIMATION_H
#define CAR_RENDER_ANIMATION_H

#include <cstdint>
#include <unordered_map>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "Game_GUI/sound/Sound.h"

#include "ServerEvent.h"

struct Animation {
    int current_frame;
    int total_frames;
    bool activated;
    int delay_counter;
};

const int DELAY = 4;

const int FRAMES_COLUMNS = 3;
const int FRAMES_AREA = 166;
const int TOTAL_FRAMES = 9;

const int NONE_ANIMATION = 0;
const int SMALL_ANIMATION = 1;
const int MEDIUM_ANIMATION = 2;
const int BIG_ANIMATION = 4;
const int EXPLOSION_ANIMATION = 9;

class CarRenderAnimation {
private:
    SDL2pp::Renderer& renderer;

    std::unordered_map<uint32_t, Animation> car_animations;

    SDL2pp::Surface animation_surf;

    SDL2pp::Texture animation_text;

    void render_frame(const Player& player, int animation_number, Coords window_left_corner,
                      int window_width, int window_high);

public:
    explicit CarRenderAnimation(SDL2pp::Renderer& renderer);

    void render_animation(const Player& player, Coords window_left_corner, int window_width,
                          int window_high);


    void clear_animations();
};

#endif
