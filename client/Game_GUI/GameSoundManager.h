#ifndef GAME_SOUND_MANAGER_H
#define GAME_SOUND_MANAGER_H

#include "../ServerEvent.h"
#include "sound/Sound.h"

const int SOUND_CHANGE = 5;
const int MAX_VOLUME = 100;
const int NO_VOLUME = 0;

const int INITIAL_VOLUME = 15;

class GameSoundManager {

private:
    Sound game_music_sdl;

    Sound game_music_lobby;

    Sound* current_music;

    bool is_muted;

    int music_volume;

public:
    GameSoundManager();

    void playLobbyMusic();

    void playGameMusic();

    void stop_music();

    void audio_configuration(const MusicConfigType& music_config);

    bool get_is_muted() const;

    int get_music_volume() const;

    void global_quit();

    ~GameSoundManager();
};


#endif
