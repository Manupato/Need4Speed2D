#include "InputHandler.h"

InputHandler::InputHandler(): last_key_send(SDLK_UNKNOWN) {}


ServerEventSender InputHandler::key_unpressed(SDL_Keycode key_pressed) {
    ServerEventSender event_send{};
    last_key_send = SDLK_UNKNOWN;
    event_send.type = ServerEventSenderType::SEND_KEY;

    switch (key_pressed) {
        case SDLK_w:
            event_send.send_key.key = DirectionKey::UP_UNPRESSED;
            break;
        case SDLK_s:
            event_send.send_key.key = DirectionKey::DOWN_UNPRESSED;
            break;
        case SDLK_a:
            event_send.send_key.key = DirectionKey::LEFT_UNPRESSED;
            break;
        case SDLK_d:
            event_send.send_key.key = DirectionKey::RIGHT_UNPRESSED;
            break;
    }

    return event_send;
}

ServerEventSender InputHandler::key_pressed(SDL_Keycode key_pressed) {
    ServerEventSender event_send{};

    if (key_pressed == last_key_send) {
        event_send.type = ServerEventSenderType::NONE;
        return event_send;
    }
    last_key_send = key_pressed;

    event_send.type = ServerEventSenderType::SEND_KEY;

    SendKey send_key{};
    MusicConfigType music_config = MusicConfigType::None;

    send_key.key = DirectionKey::None;

    switch (key_pressed) {
        case SDLK_w:
            send_key.key = DirectionKey::UP_PRESSED;
            break;
        case SDLK_s:
            send_key.key = DirectionKey::DOWN_PRESSED;
            break;
        case SDLK_a:
            send_key.key = DirectionKey::LEFT_PRESSED;
            break;
        case SDLK_d:
            send_key.key = DirectionKey::RIGHT_PRESSED;
            break;

        case SDLK_u:
            send_key.key = DirectionKey::GHOST;
            break;

        case SDLK_p:
            send_key.key = DirectionKey::LOSE;
            break;

        case SDLK_o:
            send_key.key = DirectionKey::WIN;
            break;

        case SDLK_i:
            send_key.key = DirectionKey::INFINITE_LIFE;
            break;

        case SDLK_m:
            event_send.type = ServerEventSenderType::MUSIC_CONFIG;
            music_config = MusicConfigType::MUTE;
            break;

        case SDLK_b:
            event_send.type = ServerEventSenderType::MUSIC_CONFIG;
            music_config = MusicConfigType::DECREASE;
            break;

        case SDLK_n:
            event_send.type = ServerEventSenderType::MUSIC_CONFIG;
            music_config = MusicConfigType::INCREASE;
            break;
    }

    event_send.send_key = send_key;
    event_send.music_config = music_config;

    return event_send;
}


ServerEventSender InputHandler::event_handler(const SDL_Event& input) {
    ServerEventSender event_send{};

    if (input.type == SDL_QUIT) {
        event_send.type = ServerEventSenderType::LEAVE_LOBBY;
        return event_send;
    }

    if (input.type == SDL_KEYDOWN || input.type == SDL_KEYUP) {
        if (input.key.keysym.sym == SDLK_ESCAPE || input.key.keysym.sym == SDLK_q) {
            event_send.type = ServerEventSenderType::LEAVE_LOBBY;
            return event_send;
        }
    }


    switch (input.type) {
        case SDL_KEYDOWN: {  // Se empezo a precionar una tecla
            event_send = key_pressed(input.key.keysym.sym);
            break;
        }

        case SDL_KEYUP: {  // se dejo de precionar una tecla
            event_send = key_unpressed(input.key.keysym.sym);

            break;
        }
    }

    return event_send;
}
