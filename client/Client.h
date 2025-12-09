#ifndef CLIENT_H
#define CLIENT_H

#include <string>

#include "../common/socket.h"
#include "Lobby/prelobby.h"

#include "ExceptionClient.h"
#include "ServerEvent.h"
#include "ThreadReceiver.h"
#include "ThreadSenderClient.h"


class Client {
private:
    Socket skt;

    ThreadSender thread_sender;

    ThreadReceiver thread_receiver;

    Queue<ServerEventSender>& queue_sender;

    Queue<ServerEventReceiver>& queue_receiver;

    GameSoundManager sound_manager;

    PreLobby prelobby;

    uint32_t user_id;

public:
    void execute();

    Client(int argc, char* argv[]);

    void receive_id();
};


#endif
