#ifndef RACE_PROGRESS_H
#define RACE_PROGRESS_H

// Cada jugador tendra mapeado su RaceProgress
struct RaceProgress {
    // Siempre el primer checkpoint q se busca es el 1
    int next_order = 1;
    double time_remaining_when_finished = 0;
};

#endif
