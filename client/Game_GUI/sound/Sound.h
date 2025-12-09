#ifndef TALLER_DE_PROGRAMACION_TP_GRUPAL_2025C2_GRUPO_2_SOUND_H
#define TALLER_DE_PROGRAMACION_TP_GRUPAL_2025C2_GRUPO_2_SOUND_H

#include <string>
#include <unordered_map>

#include <SDL2pp/SDL2pp.hh>
#include <SDL_mixer.h>

class Sound {

private:
    static bool inited;

    std::unordered_map<std::string, SDL2pp::Chunk> sfx{};

    std::optional<SDL2pp::Music> music_bgm;

public:
    static void init_once(int freq = 44100, Uint16 format = AUDIO_S16SYS, int channels = 2,
                          int chunksize = 1024);

    void load(const std::string& key, const std::string& path);  // WAV

    void play(const std::string& key, int volume = MIX_MAX_VOLUME);

    void set_music_volume(int volume);

    void load_music(const std::string& path);

    void play_music(int loops = -1);  // -1 = loop infinito

    void stop_music();

    bool is_music_playing() const;

    Sound() = default;

    explicit Sound(const std::string& path);

    void global_quit();

    ~Sound();
};


#endif
