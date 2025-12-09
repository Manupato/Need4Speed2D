#ifndef CLIENT_REGISTRY_H
#define CLIENT_REGISTRY_H

#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include "../../common/queue.h"
#include "../event.h"


// Al registro de clientes se le agrega un cliente cuando este se conecta. El gameloop debe
// poder enviar eventos a los clientes y para eso necesita la cola de eventos de salida
// de cada cliente. Cuando un cliente se desconecta, se lo elimina del registro.
// El registro de clientes es un recurso compartido, se necesita un monitor.
// Cada gameloop tiene su propio registro de clientes.

class ClientRegistryMonitor {
private:
    // Recurso compartido, necesita mutex.
    std::mutex m;
    // Mapa de id de cliente a su cola de eventos de salida
    std::map<int, Queue<std::shared_ptr<IEvent>>*> out_queue_sender;

public:
    ClientRegistryMonitor() = default;

    // Agrega un cliente al registro
    void add(const int id, Queue<std::shared_ptr<IEvent>>& q);

    // Elimina un cliente del registro
    void remove(const int id);

    // Envía un evento a todos los clientes
    // Seran snapshots del estado del juego en cierto momento
    void broadcast(const std::shared_ptr<IEvent>& event);

    int size();

    ~ClientRegistryMonitor() = default;

    ClientRegistryMonitor(const ClientRegistryMonitor&) = delete;
    ClientRegistryMonitor& operator=(const ClientRegistryMonitor&) = delete;
};

#endif  // CLIENT_REGISTRY_H
