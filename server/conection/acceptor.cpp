#include "acceptor.h"

Acceptor::Acceptor(const char* servname, GameManager& game_manager):
        server_socket(servname), game_manager(game_manager) {}

void Acceptor::run() {
    while (should_keep_running()) {
        try {
            reap();
            Socket peer = server_socket.accept();
            // Creamos el manejador de cliente y lo iniciamos
            auto& h = clients.emplace_back(std::move(peer), next_id++, game_manager);
            h.start();
        } catch (const LibError& e) {
            // Si el socket fue cerrado aproposito, no es un error
            if (!should_keep_running()) {
                break;
            }

            // Si fallo accept sin haber sido cerrado aproposito, ahi si es un error
            std::cerr << "Acceptor: accept() failed: " << e.what() << "\n";
            continue;
        } catch (const std::exception& e) {
            std::cerr << "Error Acceptor: " << e.what() << "\n";
        }
    }
    clear();
}

void Acceptor::stop() {
    // llamamos al stop para setear _keep_running = false;
    Thread::stop();

    // cerramos el socket para que si esta bloqueado en accept, este falle
    // y pueda salir del loop
    // Si ya estaba cerrado, seguimos con el flujo normal
    try {
        server_socket.shutdown(SHUT_RDWR);
    } catch (...) {}
    try {
        server_socket.close();
    } catch (...) {}
}

void Acceptor::reap() {
    game_manager.reap_finished_games();
    for (auto handler = clients.begin(); handler != clients.end();) {
        if (handler->is_finished()) {
            // El join es una mascara a los joins de receiver y sender
            // ya que los handlers no son threads
            handler->join();
            handler = clients.erase(handler);
        } else {
            ++handler;
        }
    }
}

void Acceptor::clear() {
    for (auto& handler: clients) handler.join();
    clients.clear();
}
