#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "../common/resource_paths.h"
#include "conection/server_logic.h"

// ./server <servicename o puerto>
int main(int argc, char* argv[]) {
    try {

        ResourcePaths::init();

        if (argc != 2) {
            std::cerr << "Bad program call. Expected " << argv[0] << " <servicename o puerto>\n";
            return EXIT_FAILURE;
        }

        ServerLogic server_logic(argv[1]);
        return server_logic.run();

    } catch (const std::exception& e) {
        std::cerr << "Something went wrong and an exception was caught: " << e.what() << "\n";
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Something went wrong and an unknown exception was caught.\n";
        return EXIT_FAILURE;
    }
}
