#include "client_registry.h"

void ClientRegistryMonitor::add(const int id, Queue<std::shared_ptr<IEvent>>& q) {
    std::lock_guard<std::mutex> lk(m);
    out_queue_sender[id] = &q;
}

void ClientRegistryMonitor::remove(const int id) {
    std::lock_guard<std::mutex> lk(m);
    out_queue_sender.erase(id);
}

// Envía un evento a todos los clientes registrados en el monitor.
void ClientRegistryMonitor::broadcast(const std::shared_ptr<IEvent>& event) {
    std::lock_guard<std::mutex> lk(m);
    for (const auto& [id, q]: out_queue_sender) {
        try {
            q->push(event);
        } catch (const ClosedQueue&) {
            // Si la cola del cliente esta cerrada, seguimos
            continue;
        }
    }
}

int ClientRegistryMonitor::size() {
    std::lock_guard<std::mutex> lk(m);
    return static_cast<int>(out_queue_sender.size());
}
