

#ifndef TALLER_DE_PROGRAMACION_TP_GRUPAL_2025C2_GRUPO_2_PLAYERSOUND_H
#define TALLER_DE_PROGRAMACION_TP_GRUPAL_2025C2_GRUPO_2_PLAYERSOUND_H

#include <chrono>
#include <vector>

#include "ServerEvent.h"
#include "Sound.h"


class PlayerSound {
private:
    Sound sound;

    std::chrono::steady_clock::time_point last_sound_time;

    int sounds_in_window = 0;

    const int TIME_WINDOW_MS = 120;  // ventana de tiempo ms

    const int MAX_SOUNDS_IN_WINDOW = 3;  // maximos sonidos al mismo tiempo

public:
    PlayerSound();

    void playSound(const std::vector<Player>& players, const Player& main_player);
};


#endif  // TALLER_DE_PROGRAMACION_TP_GRUPAL_2025C2_GRUPO_2_PLAYERSOUND_H
