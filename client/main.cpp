#include <QApplication>
#include <exception>
#include <iostream>

#include "../common/resource_paths.h"

#include "Client.h"

int main(int argc, char* argv[]) {

    ResourcePaths::init();
    QApplication app(argc, argv);

    try {
        Client client(argc, argv);
        client.execute();

        return 0;
    } catch (const ExceptionClient& e) {
        std::cerr << "Error en ejecucion del cliente: " << std::endl << e.what() << std::endl;

    } catch (const LibError& e) {
        std::cerr << "Error en la conexión al servidor: " << e.what() << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error en ejecucion del cliente (std::exception):\n" << e.what() << std::endl;

    } catch (...) {
        std::cerr << "Error no identificado en cliente" << std::endl;
    }

    return 1;
}
