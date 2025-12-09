#include "GameSoundManager.h"

#include "../../common/resource_paths.h"

GameSoundManager::GameSoundManager():
        game_music_sdl(ResourcePaths::assets() +
                       "/client_sounds/Joshua McLean - Mountain Trials.wav"),
        game_music_lobby(ResourcePaths::assets() + "/client_sounds/lobby_loop.wav"),
        current_music(nullptr),
        is_muted(false),
        music_volume(INITIAL_VOLUME) {}


void GameSoundManager::playLobbyMusic() {
    current_music = &game_music_lobby;
    Sound::init_once();

    if (!current_music->is_music_playing()) {
        current_music->set_music_volume(music_volume);
        current_music->play_music(-1);  // loop infinito
    }
}

void GameSoundManager::playGameMusic() {
    current_music = &game_music_sdl;
    Sound::init_once();

    if (!current_music->is_music_playing()) {
        current_music->set_music_volume(music_volume);
        current_music->play_music(-1);
    }
}


void GameSoundManager::stop_music() {
    if (!current_music)
        return;

    current_music->stop_music();
}

void GameSoundManager::audio_configuration(const MusicConfigType& music_config) {
    if (!current_music)
        return;

    if (music_config == MusicConfigType::MUTE) {
        is_muted = !is_muted;

        if (is_muted) {
            current_music->set_music_volume(0);

        } else {
            current_music->set_music_volume(music_volume);
        }
    } else if (music_config == MusicConfigType::DECREASE) {
        music_volume -= SOUND_CHANGE;
        is_muted = false;

        if (music_volume <= NO_VOLUME) {
            music_volume = NO_VOLUME;
            is_muted = true;
        }
        current_music->set_music_volume(music_volume);

    } else if (music_config == MusicConfigType::INCREASE) {
        music_volume += SOUND_CHANGE;
        is_muted = false;
        if (music_volume >= MAX_VOLUME) {
            music_volume = MAX_VOLUME;
        }
        current_music->set_music_volume(music_volume);
    }
}

bool GameSoundManager::get_is_muted() const { return is_muted; }

int GameSoundManager::get_music_volume() const { return music_volume; }

void GameSoundManager::global_quit() {
    game_music_sdl.global_quit();
    game_music_lobby.global_quit();
    // reset pointers
    current_music = nullptr;
}

GameSoundManager::~GameSoundManager() { Mix_Quit(); }
