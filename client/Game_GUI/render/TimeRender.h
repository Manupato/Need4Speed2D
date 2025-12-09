#ifndef TIME_RENDER_H
#define TIME_RENDER_H

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

const int SIZE_TIME = 36;
const int SIZE_COUNTDOWN = 180;

class TimeRender {
private:
    SDL2pp::Renderer& renderer;

    SDL2pp::SDLTTF ttf;

    SDL2pp::Font font;

    SDL2pp::Font big_font;

    uint32_t start_time;


    void render_countdown(int window_width, uint32_t time);

public:
    explicit TimeRender(SDL2pp::Renderer& renderer);

    void render_time(int window_width, uint32_t time);

    void set_start_time(uint32_t start_time);
};


#endif
