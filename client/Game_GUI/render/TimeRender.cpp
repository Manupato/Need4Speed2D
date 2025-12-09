#include "TimeRender.h"

#include <iomanip>
#include <sstream>
#include <string>

#include "../../../common/resource_paths.h"

TimeRender::TimeRender(SDL2pp::Renderer& renderer):
        renderer(renderer),
        ttf(),
        font(ResourcePaths::assets() + "/client_font/font_letters.ttf", SIZE_TIME),
        big_font(ResourcePaths::assets() + "/client_font/font_letters.ttf", SIZE_COUNTDOWN),
        start_time(610) {}


void TimeRender::render_countdown(int window_width, uint32_t time) {
    int countdown = time - start_time;
    std::string num = std::to_string(countdown);

    SDL_Color color = {238, 130, 238, 255};

    SDL2pp::Surface big_surface(big_font.RenderText_Solid(num, color));
    SDL2pp::Texture big_texture(renderer, big_surface);


    SDL_Rect big_dst;
    big_dst.w = big_surface.GetWidth();
    big_dst.h = big_surface.GetHeight();
    big_dst.x = (window_width - big_dst.w) / 2;
    big_dst.y = 200;

    renderer.Copy(big_texture, SDL2pp::NullOpt, big_dst);
}


void TimeRender::render_time(int window_width, uint32_t time) {
    int minutes = time / 60;
    int segs = time % 60;

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << minutes << ":" << std::setw(2) << std::setfill('0')
        << segs;
    std::string actual_time = oss.str();  // setw se usa para decir que va a haber 2 numeros, y
                                          // setfill es por si falta numero lo setea en 0

    SDL_Color color = {238, 130, 238, 255};

    SDL2pp::Surface surface(font.RenderText_Solid(actual_time, color));
    SDL2pp::Texture texture(renderer, surface);


    SDL_Rect render_rect;
    render_rect.w = surface.GetWidth();
    render_rect.h = surface.GetHeight();
    render_rect.x = (window_width - render_rect.w) / 2;
    render_rect.y = 20;

    renderer.Copy(texture, SDL2pp::NullOpt, render_rect);


    if (time >= start_time) {
        render_countdown(window_width, time);
    }
}

void TimeRender::set_start_time(uint32_t start_time) { this->start_time = start_time; }
