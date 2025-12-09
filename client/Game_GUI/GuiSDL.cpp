#include "GuiSDL.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

#include "../../common/resource_paths.h"
#include "../ExceptionClient.h"


GuiSDL::GuiSDL():
        sdl(SDL_INIT_VIDEO),
        window("Need for Speed", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
               WINDOW_INITIAL_WIDTH, WINDOW_INITIAL_HIGH, SDL_WINDOW_RESIZABLE),
        renderer(window, -1, SDL_RENDERER_ACCELERATED),
        background_texture(renderer, ResourcePaths::assets() + "/cities/balck_screen.jpeg"),
        background_texture_lv1(renderer, ResourcePaths::assets() + "/cities/balck_screen.jpeg"),
        render_car(renderer),
        render_barlife(renderer),
        render_checkpoint(renderer, window),
        render_minimap(renderer, background_texture),
        render_caranimation(renderer),
        time_render(renderer),
        arrow_render(renderer),
        mute_render(renderer),
        render_positions(renderer),
        render_carupgrades(renderer),
        start_line{{0, 0}, {0, 0}, Direction::None} {}


bool GuiSDL::get_event(SDL_Event& out) { return SDL_PollEvent(&out); }

void GuiSDL::render_top_layer(const Snapshot& snapshot, const Player& main_player,
                              Coords window_left_corner) {
    int window_width, window_high;  // Agarra tamaño ventana
    SDL_GetWindowSize(window.Get(), &window_width, &window_high);

    if (window_high < 0 || window_width < 0)
        return;


    for (const Player& player: snapshot.players) {
        if (player.car_coord_z == 1) {
            render_car.render_car(player, window_left_corner, window_width, window_high);

            render_caranimation.render_animation(player, window_left_corner, window_width,
                                                 window_high);
        }
    }

    for (const NPC& npc: snapshot.npcs) {
        if (npc.pos_z == 1) {
            render_car.render_npc(npc, window_left_corner, window_width, window_high);
        }
    }
    render_checkpoint.render_checkpoint(main_player, window_left_corner);

    render_minimap.render_minimap(window_width, snapshot.players, snapshot.npcs, main_player);

    render_barlife.render_barlife(main_player.car_life);

    arrow_render.render_arrow(main_player, window);

    time_render.render_time(window_width, snapshot.actual_time);
}

void GuiSDL::render_bottom_layer(const Snapshot& snapshot, Coords window_left_corner) {
    int window_width, window_high;  // Agarra tamaño ventana
    SDL_GetWindowSize(window.Get(), &window_width, &window_high);

    if (window_high < 0 || window_width < 0)
        return;


    render_checkpoint.render_start_line(window_left_corner, start_line);

    for (const Player& player: snapshot.players) {
        if (player.car_coord_z == 0) {
            render_car.render_car(player, window_left_corner, window_width, window_high);
            render_caranimation.render_animation(player, window_left_corner, window_width,
                                                 window_high);
        }
    }

    for (const NPC& npc: snapshot.npcs) {
        if (npc.pos_z == 0) {
            render_car.render_npc(npc, window_left_corner, window_width, window_high);
        }
    }
}


void GuiSDL::render_gameloop(const Snapshot& snapshot, const Player& main_player,
                             const bool is_muted) {
    uint32_t coord_x = main_player.player_position.coord_x;
    uint32_t coord_y = main_player.player_position.coord_y;

    int window_width, window_high;  // Agarra tamaño ventana
    SDL_GetWindowSize(window.Get(), &window_width, &window_high);

    if (window_high < 0 || window_width < 0)
        return;

    // Obtiene esquina izq de ventana
    uint32_t coord_x_left_corner = std::clamp(static_cast<int>(coord_x) - window_width / 2, 0,
                                              std::max(0, IMAGE_WIDTH - window_width));
    uint32_t coord_y_left_corner = std::clamp(static_cast<int>(coord_y) - window_high / 2, 0,
                                              std::max(0, IMAGE_HIGH - window_high));

    SDL2pp::Rect map_rendering_area(coord_x_left_corner, coord_y_left_corner, window_width,
                                    window_high);               // espacio del mapa que toma
    SDL2pp::Rect window_rect(0, 0, window_width, window_high);  // tamaño pantalla

    Coords window_left_corner = {coord_x_left_corner, coord_y_left_corner};

    renderer.Clear();
    renderer.Copy(background_texture, map_rendering_area, window_rect);


    render_bottom_layer(snapshot, window_left_corner);

    renderer.Copy(background_texture_lv1, map_rendering_area, window_rect);

    render_top_layer(snapshot, main_player, window_left_corner);

    if (is_muted) {
        mute_render.render_mute_simbol(window_width, window_high);
    }

    renderer.Present();
}


void GuiSDL::render_screen_position(const RaceResults& race_results) {
    render_positions.render_table_positions(race_results, window);
    renderer.Present();
}


void GuiSDL::render_screen_upgrades(bool clear_upgrades) {
    renderer.Clear();
    render_carupgrades.render_upgrades(window, clear_upgrades);
    renderer.Present();
}

void GuiSDL::handle_upgrades(const SDL_Event& event, CarUpgrades& current_upgrade) {
    render_carupgrades.handle_event(event, current_upgrade);
}

void GuiSDL::set_background(Map map_selected) {
    if (map_selected == Map::LibertyCity) {
        background_texture = SDL2pp::Texture(
                renderer, SDL2pp::Surface(ResourcePaths::assets() + "/cities/LC_nivel0.png"));
        background_texture_lv1 = SDL2pp::Texture(
                renderer, SDL2pp::Surface(ResourcePaths::assets() + "/cities/LC_nivel1.png"));

    } else if (map_selected == Map::SanAndreas) {
        background_texture = SDL2pp::Texture(
                renderer, SDL2pp::Surface(ResourcePaths::assets() + "/cities/SA_nivel0.png"));
        background_texture_lv1 = SDL2pp::Texture(
                renderer, SDL2pp::Surface(ResourcePaths::assets() + "/cities/SA_nivel1.png"));

    } else if (map_selected == Map::ViceCity) {
        background_texture = SDL2pp::Texture(
                renderer, SDL2pp::Surface(ResourcePaths::assets() + "/cities/VC_nivel0.png"));
        background_texture_lv1 = SDL2pp::Texture(
                renderer, SDL2pp::Surface(ResourcePaths::assets() + "/cities/VC_nivel1.png"));

    } else {
        throw ExceptionClient("Error Mapa no se encuentra entre los predefinidos");
    }
    render_caranimation.clear_animations();
}

void GuiSDL::set_start_time(uint32_t start_time) { time_render.set_start_time(start_time); }

GuiSDL::~GuiSDL() { renderer.Clear(); }
