#include "RacePhaseManager.h"

RacePhaseManager::RacePhaseManager():
        state(GameState::RUNNING_GAME),
        is_last_map(false),
        race_started(false),
        race_total_time(0),
        race_start_time(0) {}

void RacePhaseManager::process_snapshot(const Snapshot& snapshot) {
    if (snapshot.actual_time <= race_start_time) {
        race_started = true;
    }
}

void RacePhaseManager::process_pregame(const PreGame& pregame) {
    race_started = false;
    race_start_time = pregame.game_start_time;
    race_total_time = pregame.game_total_time;

    advance_phase();
}

void RacePhaseManager::process_results(const RaceResults& results) {
    state = GameState::SHOW_POSITIONS;
    is_last_map = results.is_last_race;
}

void RacePhaseManager::advance_phase() {
    if (state == GameState::SHOW_POSITIONS) {
        state = GameState::SHOW_UPGRADE;
    } else if (state == GameState::SHOW_UPGRADE) {
        state = GameState::PREGAME;
    } else if (state == GameState::PREGAME) {
        state = GameState::RUNNING_GAME;
    }
}

GameState RacePhaseManager::get_state() const { return state; }

bool RacePhaseManager::has_race_started() const { return race_started; }

bool RacePhaseManager::is_tournament_finished() const { return is_last_map; }

GameState RacePhaseManager::handle_change_fase() {

    if (is_last_map) {
        return GameState::FINISH;

    } else if (state == GameState::SHOW_POSITIONS) {
        advance_phase();
        return state;


    } else if (state == GameState::SHOW_UPGRADE) {
        advance_phase();

        return state;
    }


    return GameState::NONE;
}
