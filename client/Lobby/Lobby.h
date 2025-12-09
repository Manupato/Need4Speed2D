#ifndef LOBBY_H
#define LOBBY_H

#include <QDialog>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include "../../common/queue.h"
#include "../ServerEvent.h"

class Lobby: public QDialog {
public:
    explicit Lobby(Queue<ServerEventSender>& sender, Queue<ServerEventReceiver>& receiver,
                   QWidget* parent = nullptr);

    void receive_snapshotlobby(const Snapshot_lobby& snapshot);

    ~Lobby();

private:
    Queue<ServerEventSender>& queue_sender;
    Queue<ServerEventReceiver>& queue_receiver;

    QLabel* lobbyCodeWidget;
    QListWidget* playerList;
    QPushButton* startButton;
    QPushButton* exitButton;

    uint32_t currentLobbyCode = 0;

    bool keep_listening;

    QThread* listenerThread;

    void setupUI();
    void setupStyle();
    void setupConnections();
    void listenServer();

    void onExitLobby();

    void onStartGame();
};

#endif
