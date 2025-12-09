#include "ThreadSenderClient.h"


ThreadSender::ThreadSender(Socket& skt): socket(skt), queue_sender(), protocolo(skt) {}


void ThreadSender::run() {
    try {
        while (should_keep_running()) {
            ServerEventSender key_ingresada;  // Habria que cambiar lo de key a event
            key_ingresada = queue_sender.pop();

            protocolo.send_event(key_ingresada);
        }
    } catch (const ClosedQueue&) {
    } catch (const LibError&) {
    } catch (...) {
        std::cerr << "Error en el ThreadSender no identificado " << std::endl;
    }
}


Queue<ServerEventSender>& ThreadSender::get_sender_queue() { return queue_sender; }


void ThreadSender::stop() {
    queue_sender.close();
    Thread::stop();

    if (!socket.is_stream_send_closed()) {
        this->socket.shutdown(1);
    }
}
