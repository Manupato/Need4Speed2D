#include "ThreadReceiver.h"

#include <iostream>

#include "../common/liberror.h"

#include "ExceptionClient.h"


ThreadReceiver::ThreadReceiver(Socket& skt):
        socket(skt), queue_receiver(), protocolo(this->socket) {}


void ThreadReceiver::run() {
    try {
        bool is_socket_closed;

        while (should_keep_running()) {

            ServerEventReceiver evento = protocolo.receive_event(is_socket_closed);

            if (is_socket_closed) {
                this->stop();
            }

            queue_receiver.push(evento);
        }
    } catch (const ClosedQueue&) {
    } catch (const LibError&) {
    } catch (...) {
        std::cerr << "Error en el ThreadReceiver no identificado " << std::endl;
    }
}

void ThreadReceiver::stop() {
    Thread::stop();

    if (!socket.is_stream_recv_closed() || !socket.is_stream_send_closed()) {
        this->socket.shutdown(2);
    }
}

Queue<ServerEventReceiver>& ThreadReceiver::get_queue() { return queue_receiver; }
