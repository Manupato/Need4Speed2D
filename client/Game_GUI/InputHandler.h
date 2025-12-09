#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#include <SDL2/SDL.h>
#include <SDL2pp/SDL2pp.hh>

#include "ServerEvent.h"

class InputHandler {

private:
    SDL_Keycode last_key_send;

    ServerEventSender key_unpressed(SDL_Keycode key_pressed);

    ServerEventSender key_pressed(SDL_Keycode key_pressed);

public:
    InputHandler();

    ServerEventSender event_handler(const SDL_Event& event);
};


#endif
