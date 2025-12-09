#include "MinimalSdl.h"

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>
#include <SDL_mixer.h>

#include "../client/Game_GUI/sound/Sound.h"
#include "../common/resource_paths.h"

int main(int argc, char* argv[]) {

    // primero SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    SDL2pp::Window window("Need for Speed", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280,
                          720, SDL_WINDOW_RESIZABLE);
    SDL2pp::Renderer renderer(window, -1, SDL_RENDERER_ACCELERATED);

    ResourcePaths::init();
    Sound::init_once();

    Sound music;
    music.load_music(ResourcePaths::assets() + "/client_sounds/lobby_loop.wav");
    music.set_music_volume(1);
    music.play_music(-1);

    Sound effects;
    effects.load("boom", ResourcePaths::assets() + "/client_sounds/car_explosion.wav");
    effects.play("boom");

    effects.set_music_volume(2);

    SDL_Delay(2000);

    music.global_quit();
    SDL_Quit();

    return 0;
}
