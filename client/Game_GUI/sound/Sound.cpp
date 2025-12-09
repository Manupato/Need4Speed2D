#include "Sound.h"

#include <stdexcept>

bool Sound::inited = false;

void Sound::init_once(int freq, Uint16 format, int channels, int chunksize) {
    if (inited)
        return;
    if (Mix_OpenAudio(freq, format, channels, chunksize) != 0)
        throw std::runtime_error(std::string("Mix_OpenAudio: ") + Mix_GetError());
    Mix_AllocateChannels(16);  // reserva canales para reproducir efectos
    inited = true;
}

// extraemos archivo y nos guardamos una referencia al mismo
void Sound::load(const std::string& key, const std::string& path) {
    sfx.emplace(key, SDL2pp::Chunk(path));
}

// buscamos la referencia del archivo que queremos y lo buscamos en el mapa sfx
void Sound::play(const std::string& key, int volume) {
    auto sonido_pedido = sfx.find(key);
    if (sonido_pedido == sfx.end())
        return;
    // Mix_PlayChannel devuelve el numero del canal usado
    int channel = Mix_PlayChannel(-1, sonido_pedido->second.Get(), 0);
    if (channel != -1) {
        Mix_Volume(channel, volume);
    }
}

void Sound::set_music_volume(int volume) { Mix_VolumeMusic(volume); }


void Sound::load_music(const std::string& path) {
    if (music_bgm) {
        Mix_HaltMusic();    // detiene la música que estaba sonando
        music_bgm.reset();  // destruye el objeto SDL2pp::Music previo
    }
    music_bgm.emplace(path);  // carga la nueva música
}

void Sound::play_music(int loops) {
    if (!music_bgm)
        return;
    // inicializa un hilo interno de musica que va a durar lo que diga el loop, -1 = infinito
    if (Mix_PlayMusic(music_bgm->Get(), loops) != 0) {
        throw std::runtime_error(std::string("Mix_PlayMusic: ") + Mix_GetError());
    }
}


void Sound::stop_music() { Mix_HaltMusic(); }


bool Sound::is_music_playing() const { return Mix_PlayingMusic() != 0; }


Sound::Sound(const std::string& path) {
    init_once();
    load_music(path);
}

void Sound::global_quit() {
    if (inited) {
        stop_music();  // si hay música sonando
        Mix_CloseAudio();
        inited = false;
    }
    sfx.clear();  // limpia todos los chunks
}

Sound::~Sound() {
    stop_music();  // por si hay música corriendo
    sfx.clear();
    music_bgm.reset();  // destruye la música
    if (inited) {
        Mix_CloseAudio();
        inited = false;
    }
}
