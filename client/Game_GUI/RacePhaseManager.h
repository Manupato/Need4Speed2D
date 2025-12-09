#ifndef RACE_PHASE_MANAGER_H
#define RACE_PHASE_MANAGER_H

#include "ServerEvent.h"

enum class GameState { PREGAME, RUNNING_GAME, SHOW_POSITIONS, SHOW_UPGRADE, FINISH, NONE };

class RacePhaseManager {
private:
    GameState state;
    bool is_last_map;
    bool race_started;

    uint32_t race_total_time;
    uint32_t race_start_time;

public:
    RacePhaseManager();

    void process_snapshot(const Snapshot& snapshot);

    void process_pregame(const PreGame& pregame);

    void process_results(const RaceResults& results);

    void advance_phase();

    GameState get_state() const;

    bool has_race_started() const;

    bool is_tournament_finished() const;

    GameState handle_change_fase();
};

#endif
