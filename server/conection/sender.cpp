#include "sender.h"

Sender::Sender(Socket& peer_socket, const int id, Queue<std::shared_ptr<IEvent>>& queue_out):
        peer(peer_socket), id_(id), queue_out(queue_out) {}

void Sender::run() {
    try {
        // Ni bien se establece, una conexion, le notificamos al cliente su id
        // Asi a futuro en snapshots, puede identificarse.
        protocol.send_id_to_client(peer, id_);
        bool continue_running = true;
        while (continue_running) {
            continue_running = protocol.send_event_to_client(peer, queue_out);
        }
    } catch (const ClosedQueue&) {
        // Esto no es un error, es la forma que tiene de cerrar la cola.
    } catch (const std::exception& e) {
        std::cerr << "Sender exception: " << e.what() << "\n";
    } catch (...) {
        std::cerr << "Sender unexpected exception\n";
    }
}

void Sender::close_queue() {
    try {
        queue_out.close();
    } catch (const std::exception&) {
        // Si ya estaba cerrada, no hacemos nada
    }
}
