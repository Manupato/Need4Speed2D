#ifndef BUTTON_H
#define BUTTON_H

#include <string>

#include <SDL2pp/SDL2pp.hh>

const int MARGIN_Y = 12;

const int SMALL_FONT = 15;
const int FONT = 36;
class Button {
    SDL2pp::Renderer& renderer;
    SDL2pp::Rect button_rect;

    std::string label;
    std::string sublabel;

    SDL_Color text_color;
    SDL2pp::Font font;
    SDL2pp::Font subfont;

    bool hovered;
    bool pressed;

    bool selected;


public:
    explicit Button(SDL2pp::Renderer& renderer);

    void setProperties(int x, int y, int w, int h, const std::string& new_label,
                       const std::string& new_sublabel);

    void update(const SDL_Event& e);

    void render();

    bool isClicked(const SDL_Event& e);

    std::string getLabel() const { return label; }

    void setSelected(bool val) { selected = val; }

    bool isSelected() const { return selected; }
};

#endif
