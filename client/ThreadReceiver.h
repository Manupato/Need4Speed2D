#ifndef THREAD_RECEIVER_H
#define THREAD_RECEIVER_H

#include "../common/queue.h"
#include "../common/socket.h"
#include "../common/thread.h"

#include "ProtocolClient.h"
#include "ServerEvent.h"


class ThreadReceiver: public Thread {
private:
    Socket& socket;

    Queue<ServerEventReceiver> queue_receiver;

    ProtocolClient protocolo;

public:
    explicit ThreadReceiver(Socket& skt);

    void run() override;

    void stop() override;

    Queue<ServerEventReceiver>& get_queue();
};

#endif
