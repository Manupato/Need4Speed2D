#include "receiver.h"

#include "client_handler.h"

Receiver::Receiver(Socket& peer_socket, const int id, ClientHandler& ch):
        peer(peer_socket), id(id), client_handler(ch) {}

void Receiver::run() {
    try {
        while (true) {

            switch (protocol.get_type_of_command(peer)) {
                case CommandReceiverType::Move: {
                    handle_move_command();
                    break;
                }
                case CommandReceiverType::JoinLobby: {
                    handle_join_lobby();
                    break;
                }
                case CommandReceiverType::CreateLobby: {
                    handle_create_lobby();
                    break;
                }
                case CommandReceiverType::StartLobby: {
                    handle_start_lobby();
                    break;
                }
                case CommandReceiverType::Disconect: {
                    client_handler.disconnect();
                    break;
                }
                case CommandReceiverType::NewCar: {
                    break;
                }
                case CommandReceiverType::Upgrade: {
                    handle_upgrade_command();
                    break;
                }
                case CommandReceiverType::DefiniteDisconect: {
                    client_handler.disconnect();
                    return;
                }
                default: {
                    std::cerr << "Receiver: Comando desconocido recibido por el cliente"
                              << std::endl;
                    return;
                }
            }
        }
    } catch (const ClosedQueue&) {
        // Esto no es un error, es la forma que tiene de cerrar la cola.
    } catch (const std::exception& e) {
        std::cerr << "Receiver exception: " << e.what() << "\n";
    } catch (...) {
        std::cerr << "Receiver unexpected exception\n";
    }
}

void Receiver::handle_move_command() {
    CommandReceiver cmd = protocol.get_command_move(peer, id);
    // De todavia no tener queue del gameloop, lo mantendra en null hasta que exista
    queue_gameloop = client_handler.get_queue_gameloop();
    if (queue_gameloop) {
        try {
            queue_gameloop->push(cmd);
        } catch (const ClosedQueue&) {
            // Justo la partida termino mientras pusheabamos, ignoramos el input.
        }
    }
}

void Receiver::handle_upgrade_command() {
    CommandReceiver cmd = protocol.get_command_upgrade(peer, id);
    queue_gameloop = client_handler.get_queue_gameloop();

    if (queue_gameloop) {
        try {
            queue_gameloop->push(cmd);
        } catch (const ClosedQueue&) {
            // Justo la partida termino mientras pusheabamos, ignoramos el input.
        }
    }
}

void Receiver::handle_join_lobby() {
    auto cmd = protocol.get_command_join_lobby(peer, id);
    client_handler.join_lobby(cmd);
}

void Receiver::handle_create_lobby() {
    auto cmd = protocol.get_command_create_lobby(peer, id);
    client_handler.create_lobby(cmd);
}

void Receiver::handle_start_lobby() {
    client_handler.start_lobby(protocol.get_command_start_lobby(peer, id).lobby_id);
    queue_gameloop = client_handler.get_queue_gameloop();
}
