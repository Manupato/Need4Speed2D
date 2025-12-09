#include "RenderCarUpgrades.h"

#include <string>
#include <utility>

#include "../../../common/resource_paths.h"

RenderCarUpgrades::RenderCarUpgrades(SDL2pp::Renderer& renderer):
        renderer(renderer),
        background_label_img(ResourcePaths::assets() + "/client_img_render/upgrades.png"),
        background_label(renderer, background_label_img),
        background_img(ResourcePaths::assets() + "/client_img_render/fondo_upgrades.jpg"),
        background(renderer, background_img),
        needes_resize(true) {}


void RenderCarUpgrades::render_upgrades(SDL2pp::Window& window, bool clear_upgrades) {
    int window_w, window_h;
    SDL_GetWindowSize(window.Get(), &window_w, &window_h);

    int table_x = (window_w - background_label_img.GetWidth()) / 2;
    int table_y = (window_h - background_label_img.GetHeight()) / 2;

    renderer.Clear();

    SDL2pp::Rect background_rect = {0, 0, background_img.GetWidth(), background_img.GetHeight()};
    renderer.Copy(background, SDL2pp::NullOpt, background_rect);

    SDL2pp::Rect background_label_rect = {table_x, table_y, background_label_img.GetWidth(),
                                          background_label_img.GetHeight()};
    renderer.Copy(background_label, SDL2pp::NullOpt, background_label_rect);


    if (needes_resize) {
        upgrade_buttons.clear();
        create_buttons(table_x, table_y);
        needes_resize = false;
    }

    for (auto& button: upgrade_buttons) {
        button.render();
        if (clear_upgrades) {
            button.setSelected(false);
        }
    }
}


void RenderCarUpgrades::create_buttons(int table_x, int table_y) {
    std::string lable;
    std::string sublable;
    int pos_x = 64;
    int pos_y;

    for (int i = 0; i < 9; i++) {
        Button button(renderer);

        if (i < 3) {
            lable = "Manejabilidad ";
            pos_y = 134;
        } else if (i < 6) {
            lable = "Velocidad ";
            pos_y = 194;
        } else {
            lable = "Escudo ";
            pos_y = 254;
        }

        int btn_x = table_x + pos_x;
        int btn_y = table_y + pos_y;

        if ((i % 3) == 0) {
            lable.append("I");
            sublable = "+10 s";
        } else if ((i % 3) == 1) {
            lable.append("II");
            sublable = "+20 s";
        } else {
            lable.append("III");
            sublable = "+30 s";
        }

        button.setProperties(btn_x + MARGIN * (i % 3) + BTN_WIDTH * (i % 3), btn_y + MARGIN,
                             BTN_WIDTH, BTN_HEIGHT, lable, sublable);
        button.render();
        upgrade_buttons.push_back(std::move(button));
    }
}


void RenderCarUpgrades::handle_event(const SDL_Event& e, CarUpgrades& current_upgrade) {

    if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
        needes_resize = true;
    }

    for (auto& upgrade_button: upgrade_buttons) {
        upgrade_button.update(e);

        if (upgrade_button.isClicked(e)) {

            for (auto& button: upgrade_buttons) {
                button.setSelected(false);
            }
            upgrade_button.setSelected(true);

            if (upgrade_button.getLabel() == "Manejabilidad I")
                current_upgrade = CarUpgrades::DRIVEABILITY_I;
            else if (upgrade_button.getLabel() == "Manejabilidad II")
                current_upgrade = CarUpgrades::DRIVEABILITY_II;
            else if (upgrade_button.getLabel() == "Manejabilidad III")
                current_upgrade = CarUpgrades::DRIVEABILITY_III;
            else if (upgrade_button.getLabel() == "Velocidad I")
                current_upgrade = CarUpgrades::VELOCITY_I;
            else if (upgrade_button.getLabel() == "Velocidad II")
                current_upgrade = CarUpgrades::VELOCITY_II;
            else if (upgrade_button.getLabel() == "Velocidad III")
                current_upgrade = CarUpgrades::VELOCITY_III;
            else if (upgrade_button.getLabel() == "Escudo I")
                current_upgrade = CarUpgrades::SHIELD_I;
            else if (upgrade_button.getLabel() == "Escudo II")
                current_upgrade = CarUpgrades::SHIELD_II;
            else if (upgrade_button.getLabel() == "Escudo III")
                current_upgrade = CarUpgrades::SHIELD_III;
        }
    }
}
