#include "GameloopRace.h"


GameloopRace::GameloopRace(Queue<ServerEventSender>& queue_sender,
                           Queue<ServerEventReceiver>& queue_receiver, uint32_t user_id,
                           GameSoundManager& music_manager):
        queue_sender(queue_sender),
        queue_receiver(queue_receiver),
        _keep_running(true),
        gui_sdl(),
        user_id(user_id),
        current_upgrade(CarUpgrades::NOTHING),
        music_manager(music_manager),
        playerSound(),
        input_handlers(),
        race_manager() {}


void GameloopRace::handle_change_phase() {
    GameState state = race_manager.handle_change_fase();

    if (state == GameState::FINISH) {
        _keep_running = false;

    } else if (state == GameState::SHOW_UPGRADE) {
        gui_sdl.render_screen_upgrades(true);

    } else if (state == GameState::PREGAME) {
        ServerEventSender event;
        event.type = ServerEventSenderType::UPGRADES;
        event.car_upgrade = current_upgrade;
        queue_sender.try_push(event);

        current_upgrade = CarUpgrades::NOTHING;
    }
}


void GameloopRace::handle_snapshot(const Snapshot& snapshot) {

    auto it = std::find_if(snapshot.players.begin(), snapshot.players.end(),
                           [&](const Player& p) { return p.user_id == this->user_id; });

    Player main_player;
    if (it != snapshot.players.end()) {
        main_player = *it;
    }

    race_manager.process_snapshot(snapshot);

    gui_sdl.render_gameloop(snapshot, main_player, music_manager.get_is_muted());
    if (!music_manager.get_is_muted()) {
        playerSound.playSound(snapshot.players, main_player);
    }
}


void GameloopRace::update_game_state(const ServerEventReceiver& event) {
    switch (event.type) {
        case ServerEventReceiverType::SNAPSHOT:
            handle_snapshot(event.snapshot);
            break;
        case ServerEventReceiverType::PREGAME:
            gui_sdl.set_background(event.pre_snapshot.map_selected);
            race_manager.process_pregame(event.pre_snapshot);

            gui_sdl.set_start_line(event.pre_snapshot.start_line);
            gui_sdl.set_start_time(event.pre_snapshot.game_start_time);

            break;
        case ServerEventReceiverType::RACE_RESULTS:
            race_manager.process_results(event.race_result);
            gui_sdl.render_screen_position(event.race_result);

            break;
        case ServerEventReceiverType::CHANGE_FASE:
            handle_change_phase();
            break;
        default:
            break;
    }
}


void GameloopRace::input_handler(const SDL_Event& input) {

    if (race_manager.get_state() == GameState::SHOW_UPGRADE) {
        gui_sdl.handle_upgrades(input, current_upgrade);
    }
    ServerEventSender event = input_handlers.event_handler(input);

    if (event.type == ServerEventSenderType::LEAVE_LOBBY) {
        _keep_running = false;
        queue_sender.try_push(event);

    } else if (race_manager.has_race_started() && event.type == ServerEventSenderType::SEND_KEY) {
        queue_sender.try_push(event);

    } else if (event.type == ServerEventSenderType::MUSIC_CONFIG) {
        music_manager.audio_configuration(event.music_config);
    }
}


void GameloopRace::run() {
    try {
        music_manager.playGameMusic();

        ServerEventReceiver event;
        while (_keep_running) {

            SDL_Event last_input;
            while (gui_sdl.get_event(last_input)) {
                input_handler(last_input);
            }

            event.type = ServerEventReceiverType::ERROR;
            while (queue_receiver.try_pop(event) &&
                   event.type == ServerEventReceiverType::SNAPSHOT) {}

            if (event.type != ServerEventReceiverType::ERROR) {
                update_game_state(event);
            }

            if (race_manager.get_state() == GameState::SHOW_UPGRADE) {
                gui_sdl.render_screen_upgrades(false);
            }


            SDL_Delay(16);
        }

        music_manager.stop_music();

    } catch (const SDL2pp::Exception& e) {
        std::cerr << "Error en ejecucion del cliente: " << std::endl << e.what() << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error en ejecucion del cliente (std::exception):\n" << e.what() << std::endl;

    } catch (...) {
        std::cerr << "Error no identificado en cliente" << std::endl;
    }
}
