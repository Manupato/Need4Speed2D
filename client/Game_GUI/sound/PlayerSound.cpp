#include "PlayerSound.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>

#include "../../../common/resource_paths.h"

PlayerSound::PlayerSound() {
    sound.load("break", ResourcePaths::assets() + "/client_sounds/frenada.wav");
    sound.load("finish", ResourcePaths::assets() + "/client_sounds/llegada.wav");
    sound.load("crash", ResourcePaths::assets() + "/client_sounds/crash_sound.wav");
    sound.load("explosion", ResourcePaths::assets() + "/client_sounds/car_explosion.wav");

    last_sound_time = std::chrono::steady_clock::now();
}


void PlayerSound::playSound(const std::vector<Player>& players, const Player& main_player) {

    const float MAX_HEAR_DISTANCE = 500.0f;  // maxima distancia a la que un usuario puede escuchar


    auto now = std::chrono::steady_clock::now();  // tiempo actual
    int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_sound_time)
                          .count();  // tiempo que paso desde now

    if (elapsed > TIME_WINDOW_MS) {
        sounds_in_window = 0;
        last_sound_time = now;
    }

    for (const auto& player: players) {
        std::string key;

        if (player.type_sound == TypeSound::STOP) {
            key = "break";

        } else if (player.type_sound == TypeSound::FINISH) {
            key = "finish";

        } else if (player.car_animation > 0 && player.car_animation < 4) {
            key = "crash";
        } else if (player.car_animation == 4) {
            key = "explosion";
        } else {
            continue;
        }

        // posición en la que esta el usuario cuando hace el sonido
        float x = static_cast<float>(player.player_position.coord_x);
        float y = static_cast<float>(player.player_position.coord_y);

        // posición del jugador principal
        float mx = static_cast<float>(main_player.player_position.coord_x);
        float my = static_cast<float>(main_player.player_position.coord_y);

        float distance_player_mainPayerX = x - mx;
        float distance_player_mainPayerY = y - my;

        float distancia_real = std::sqrt(distance_player_mainPayerX * distance_player_mainPayerX +
                                         distance_player_mainPayerY * distance_player_mainPayerY);

        // genera numero de cuan cerca esta a vos
        float distance_porcentege = 1.0f - (distancia_real / MAX_HEAR_DISTANCE);
        distance_porcentege = std::clamp(distance_porcentege, 0.0f,
                                         1.0f);  // el valor no puede ser uno del limite

        // convierte el porcentaje de 0 a 1 al volumen especifico
        int volume = static_cast<int>(distance_porcentege * MIX_MAX_VOLUME);

        if (volume <= 0) {
            continue;
        }

        if (sounds_in_window >= MAX_SOUNDS_IN_WINDOW) {
            continue;
        }

        sound.play(key, volume);
        sounds_in_window++;
    }
}
