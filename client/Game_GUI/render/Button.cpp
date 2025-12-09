#include "Button.h"

#include "../../../common/resource_paths.h"

Button::Button(SDL2pp::Renderer& renderer):
        renderer(renderer),
        button_rect(0, 0, 0, 0),
        label(""),
        text_color({255, 105, 180, 255}),  // color rosa
        font(ResourcePaths::assets() + "/client_font/font_letters.ttf", FONT),
        subfont(ResourcePaths::assets() + "/client_font/font_letters.ttf", SMALL_FONT),
        hovered(false),
        pressed(false),
        selected(false) {}


void Button::setProperties(int x, int y, int w, int h, const std::string& new_label,
                           const std::string& new_sublabel) {
    button_rect = SDL2pp::Rect(x, y, w, h);
    label = new_label;
    sublabel = new_sublabel;
}


void Button::update(const SDL_Event& e) {
    int pos_x, pos_y;
    SDL_GetMouseState(&pos_x, &pos_y);

    SDL_Point p{pos_x, pos_y};
    hovered = SDL_PointInRect(&p, &button_rect);

    if (hovered && e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        pressed = true;
    }

    if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
        pressed = false;
    }
}


void Button::render() {

    if (pressed || selected) {
        renderer.SetDrawColor(120, 120, 120, 255);  // gris claro
    } else if (hovered) {
        renderer.SetDrawColor(100, 100, 100, 255);  // gris medio
    } else {
        renderer.SetDrawColor(80, 80, 80, 255);  // gris oscuro
    }

    renderer.FillRect(button_rect);

    if (!label.empty()) {
        SDL2pp::Surface text_surf(font.RenderText_Solid(label, text_color));
        SDL2pp::Texture text_tex(renderer, text_surf);

        int text_w = text_surf.GetWidth();
        int text_h = text_surf.GetHeight();

        int text_x = button_rect.GetX() + (button_rect.GetW() - text_w) / 2;
        int text_y = button_rect.GetY() + (button_rect.GetH() - text_h) / 2;

        renderer.Copy(text_tex, SDL2pp::NullOpt, SDL2pp::Rect(text_x, text_y, text_w, text_h));
    }


    if (!sublabel.empty()) {
        SDL_Color sub_color = {255, 50, 50, 255};  // rojo
        SDL2pp::Surface sub_surf(subfont.RenderText_Solid(sublabel, sub_color));
        SDL2pp::Texture sub_tex(renderer, sub_surf);

        int sub_w = sub_surf.GetWidth();
        int sub_h = sub_surf.GetHeight();

        int sub_x = button_rect.GetX() + (button_rect.GetW() - sub_w) / 2;
        int sub_y = button_rect.GetY() + (button_rect.GetH() / 2) + MARGIN_Y;

        renderer.Copy(sub_tex, SDL2pp::NullOpt, SDL2pp::Rect(sub_x, sub_y, sub_w, sub_h));
    }
}

bool Button::isClicked(const SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
        int mouse_x = e.button.x;
        int mouse_y = e.button.y;
        SDL_Point point{mouse_x, mouse_y};
        return SDL_PointInRect(&point, &button_rect);
    }
    return false;
}
