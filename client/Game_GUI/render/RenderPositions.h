#ifndef RENDER_POSITIONS_H
#define RENDER_POSITIONS_H

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "ServerEvent.h"

const int FONT_SIZE = 36;


const int INDEX_NAME_COLUMN = 109;
const int INDEX_RACE_COLUMN = 334;
const int INDEX_TOTAL_COLUMN = 428;


class RenderPositions {
private:
    SDL2pp::Renderer& renderer;

    SDL2pp::Font font;

    SDL2pp::Surface table_img;

    SDL2pp::Texture table_texture;

    int last_pos_show;

    int get_y_position(int position);

    void render_normal_positions(const RaceResults& race_results, int table_x, int table_y);

    void render_final_positions(const RaceResults& race_results, int table_x, int table_y);

    void render_player(const PlayerResults& player, int y, int table_x);

public:
    explicit RenderPositions(SDL2pp::Renderer& renderer);

    void render_table_positions(const RaceResults& race_results, SDL2pp::Window& window);
};

#endif
