#ifndef GAMELOOPRACE_H
#define GAMELOOPRACE_H

#include "../../common/queue.h"
#include "sound/PlayerSound.h"

#include "GameSoundManager.h"
#include "GuiSDL.h"
#include "InputHandler.h"
#include "RacePhaseManager.h"

class GameloopRace {

private:
    Queue<ServerEventSender>& queue_sender;

    Queue<ServerEventReceiver>& queue_receiver;

    bool _keep_running;

    GuiSDL gui_sdl;

    uint32_t user_id;

    CarUpgrades current_upgrade;

    GameSoundManager& music_manager;

    PlayerSound playerSound;

    InputHandler input_handlers;

    RacePhaseManager race_manager;


    void handle_snapshot(const Snapshot& snapshot);

    void update_game_state(const ServerEventReceiver& event);

    void input_handler(const SDL_Event& event);

    void handle_change_phase();

public:
    GameloopRace(Queue<ServerEventSender>& queue_sender, Queue<ServerEventReceiver>& queue_receiver,
                 uint32_t user_id, GameSoundManager& music_manager);


    void run();
};

#endif
