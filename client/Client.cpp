#include "Client.h"

#include "Game_GUI/GameloopRace.h"
#include "Lobby/Lobby.h"

Client::Client(int argc, char* argv[]):
        skt(argv[1], argv[2]),
        thread_sender(skt),
        thread_receiver(skt),
        queue_sender(thread_sender.get_sender_queue()),
        queue_receiver(thread_receiver.get_queue()),
        sound_manager(),
        prelobby(queue_sender, queue_receiver, sound_manager),
        user_id(0) {
    if (argc != 3) {
        throw ExceptionClient("Client::Client(): Cantidad de argumentos incorrectos, debe ser de "
                              "la forma: ./client host puerto");
    }
}

void Client::receive_id() {

    ServerEventReceiver event = queue_receiver.pop();

    if (event.type == ServerEventReceiverType::RECEIVE_ID) {
        this->user_id = event.id_jugador;
    } else {
        throw ExceptionClient("Error: socket desfazado, no se recibio el id");
    }
}


void Client::execute() {
    thread_sender.start();
    thread_receiver.start();

    receive_id();

    while (true) {
        prelobby.start_music();
        prelobby.exec();
        auto eleccion = prelobby.resultAction();

        if (eleccion == Action::None) {
            sound_manager.global_quit();
            break;  // salimos del bucle principal
        }

        GameloopRace gameloop_race(queue_sender, queue_receiver, user_id, sound_manager);
        gameloop_race.run();

        sound_manager.global_quit();
    }

    thread_receiver.stop();
    thread_sender.stop();

    thread_receiver.join();
    thread_sender.join();
}
