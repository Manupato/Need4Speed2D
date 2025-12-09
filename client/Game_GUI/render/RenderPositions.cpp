#include "RenderPositions.h"

#include <algorithm>  // para std::sort
#include <iomanip>
#include <sstream>

#include "../../../common/resource_paths.h"


RenderPositions::RenderPositions(SDL2pp::Renderer& renderer):
        renderer(renderer),
        font(ResourcePaths::assets() + "/client_font/font_letters.ttf", FONT_SIZE),
        table_img(ResourcePaths::assets() + "/client_img_render/table_positions.png"),
        table_texture(renderer, table_img),
        last_pos_show(0) {}


void RenderPositions::render_table_positions(const RaceResults& race_results,
                                             SDL2pp::Window& window) {
    int window_w, window_h;
    SDL_GetWindowSize(window.Get(), &window_w, &window_h);

    int table_x = (window_w - table_img.GetWidth()) / 2;
    int table_y = (window_h - table_img.GetHeight()) / 2;

    renderer.Copy(table_texture, SDL2pp::NullOpt,
                  SDL2pp::Rect(table_x, table_y, table_img.GetWidth(), table_img.GetHeight()));

    if (race_results.is_last_race) {
        render_final_positions(race_results, table_x, table_y);
    } else {
        render_normal_positions(race_results, table_x, table_y);
    }
}

int RenderPositions::get_y_position(int position) {
    int pos_y;

    if (position == 0) {
        pos_y = 53;

    } else if (position == 1) {
        pos_y = 90;

    } else if (position == 2) {
        pos_y = 126;

    } else if (position == 3) {
        pos_y = 162;

    } else if (position == 4) {
        pos_y = 200;

    } else if (position == 5) {
        pos_y = 237;

    } else if (position == 6) {
        pos_y = 273;

    } else {
        pos_y = 309;
    }
    return pos_y;
}


void RenderPositions::render_normal_positions(const RaceResults& race_results, int table_x,
                                              int table_y) {
    const auto& players = race_results.players;

    for (size_t i = 0; i < players.size(); ++i) {
        const PlayerResults& p = players[i];
        int pos_y = get_y_position(i);
        int y = table_y + pos_y;

        render_player(p, y, table_x);
    }
}


void RenderPositions::render_final_positions(const RaceResults& race_results, int table_x,
                                             int table_y) {
    const auto& players = race_results.players;

    for (size_t i = 0; i < players.size(); ++i) {
        const PlayerResults& p = players[i];
        int pos_y = get_y_position(i + 3);
        int y = table_y + pos_y;

        render_player(p, y, table_x);
    }

    int amount;

    if (last_pos_show != 3) {
        amount = 0;
        if (race_results.actual_phase == ShowPlayerPhase::THIRD_PLACE &&
            race_results.podium_players[2].exist) {
            amount = 1;
        } else if (race_results.actual_phase == ShowPlayerPhase::SECOND_PLACE &&
                   race_results.podium_players[1].exist) {
            amount = 2;
        } else if (race_results.actual_phase == ShowPlayerPhase::FIRST_PLACE) {
            amount = 3;
        }

        if (amount == 0 && players.empty()) {
            if (race_results.podium_players[2].exist) {
                amount = 1;
            } else if (race_results.podium_players[1].exist) {
                amount = 2;
            } else {
                amount = 3;
            }
        }

        if (last_pos_show == amount && amount < 3 &&
            race_results.actual_phase != ShowPlayerPhase::REGULAR) {
            amount++;
        }
        last_pos_show = amount;

    } else {
        amount = 3;
    }

    for (int i = amount; i > 0; i--) {
        if (i == 1 && race_results.podium_players[2].exist) {
            render_player(race_results.podium_players[2], table_y + get_y_position(2), table_x);
        } else if (i == 2 && race_results.podium_players[1].exist) {
            render_player(race_results.podium_players[1], table_y + get_y_position(1), table_x);
        } else if (i == 3) {
            render_player(race_results.podium_players[0], table_y + get_y_position(0), table_x);
        }
    }
}

void RenderPositions::render_player(const PlayerResults& p, int y, int table_x) {
    SDL_Color color = {255, 255, 255, 255};

    // Nombre del jugador
    std::ostringstream player_name;
    player_name << p.player_name;
    SDL2pp::Surface surf_name(font.RenderText_Blended(player_name.str(), color));
    SDL2pp::Texture tex_name(renderer, surf_name);
    renderer.Copy(tex_name, SDL2pp::NullOpt,
                  SDL2pp::Rect(table_x + INDEX_NAME_COLUMN, y, surf_name.GetWidth(),
                               surf_name.GetHeight()));

    // Tiempo actual
    std::ostringstream race_time;
    race_time << std::setw(2) << std::setfill('0') << (p.last_race_time / 60) << ":" << std::setw(2)
              << std::setfill('0') << (p.last_race_time % 60);
    SDL2pp::Surface surf_race(font.RenderText_Blended(race_time.str(), color));
    SDL2pp::Texture tex_race(renderer, surf_race);
    renderer.Copy(tex_race, SDL2pp::NullOpt,
                  SDL2pp::Rect(table_x + INDEX_RACE_COLUMN, y, surf_race.GetWidth(),
                               surf_race.GetHeight()));


    // Tiempo acumulado
    std::ostringstream race_total;
    race_total << std::setw(2) << std::setfill('0') << (p.total_race_time / 60) << ":"
               << std::setw(2) << std::setfill('0') << (p.total_race_time % 60);
    SDL2pp::Surface surf_total(font.RenderText_Blended(race_total.str(), color));
    SDL2pp::Texture tex_total(renderer, surf_total);
    renderer.Copy(tex_total, SDL2pp::NullOpt,
                  SDL2pp::Rect(table_x + INDEX_TOTAL_COLUMN, y, surf_total.GetWidth(),
                               surf_total.GetHeight()));
}
