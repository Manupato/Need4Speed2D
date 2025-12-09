#ifndef THREAD_SENDER_H
#define THREAD_SENDER_H

#include "../common/liberror.h"
#include "../common/queue.h"
#include "../common/socket.h"
#include "../common/thread.h"

#include "ProtocolClient.h"


class ThreadSender: public Thread {
private:
    Socket& socket;

    Queue<ServerEventSender> queue_sender;

    ProtocolClient protocolo;

public:
    explicit ThreadSender(Socket& skt);

    void run() override;

    void stop() override;

    Queue<ServerEventSender>& get_sender_queue();
};

#endif
