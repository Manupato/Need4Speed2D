#include "server_logic.h"

ServerLogic::ServerLogic(const char* service_or_port):
        game_manager(), acceptor(service_or_port, game_manager) {}

int ServerLogic::run() {
    acceptor.start();

    int ch;
    while ((ch = std::cin.get()) != EOF) {
        if (ch == 'q')
            break;
    }
    return EXIT_SUCCESS;
}

ServerLogic::~ServerLogic() {
    acceptor.stop();
    acceptor.join();

    game_manager.stop_all();
    game_manager.join_all();
}
