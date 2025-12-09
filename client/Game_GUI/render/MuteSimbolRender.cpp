#include "MuteSimbolRender.h"

#include "../../../common/resource_paths.h"


MuteSimbolRender::MuteSimbolRender(SDL2pp::Renderer& renderer):
        renderer(renderer),
        mute_img(ResourcePaths::assets() + "/client_img_render/mute_img.png"),
        mute_text(renderer, mute_img),
        src_rect(SRC_MUTE_POS, SRC_MUTE_POS, mute_img.GetWidth(), mute_img.GetHeight()) {}

void MuteSimbolRender::render_mute_simbol(int window_width, int window_high) {

    SDL2pp::Rect position_rect(window_width - 64, window_high - 64, MUTE_SIZE, MUTE_SIZE);
    renderer.Copy(mute_text, src_rect, position_rect);
}
