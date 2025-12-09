#include "Lobby.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QThread>
#include <iostream>
#include <vector>

Lobby::Lobby(Queue<ServerEventSender>& sender, Queue<ServerEventReceiver>& receiver,
             QWidget* parent):
        QDialog(parent), queue_sender(sender), queue_receiver(receiver), keep_listening(true) {

    setWindowTitle("Partida — Esperando jugadores...");
    resize(720, 420);

    setupUI();
    setupStyle();
    setupConnections();

    listenerThread = QThread::create([this]() { listenServer(); });

    connect(listenerThread, &QThread::finished, listenerThread, &QObject::deleteLater);
    listenerThread->start();
}


void Lobby::setupUI() {
    // organizador de widget principal
    auto* root = new QVBoxLayout(this);

    // widget de partida
    lobbyCodeWidget = new QLabel("Código de la partida: ---", this);
    lobbyCodeWidget->setAlignment(Qt::AlignCenter);
    lobbyCodeWidget->setStyleSheet("color:#FF3399; font-weight:600;");

    // widget de mostrar Jugadores
    auto* players = new QLabel("Jugadores en la sala:", this);
    players->setStyleSheet("color:#FF3399; font-weight:600;");

    // lista con jugadores que se van a ir mostrando
    playerList = new QListWidget(this);
    playerList->setSelectionMode(QAbstractItemView::NoSelection);

    // Botones
    startButton = new QPushButton("Iniciar partida", this);
    startButton->setObjectName("btnPrimary");

    exitButton = new QPushButton("Salir", this);
    exitButton->setObjectName("btnSecondary");

    // pongo widgets de manera especifica
    root->addWidget(lobbyCodeWidget);
    root->addSpacing(10);
    root->addWidget(players);
    root->addWidget(playerList, 1);
    root->addSpacing(10);

    auto* buttons = new QHBoxLayout();
    buttons->addStretch();
    buttons->addWidget(exitButton);
    buttons->addWidget(startButton);
    root->addLayout(buttons);
}


void Lobby::setupStyle() {
    setStyleSheet(R"(
        QDialog {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #111727, stop:1 #0a0f1d);
            font-family: "Poppins", "Segoe UI";
            color: #FF3399; /* texto global rosa */
            font-size: 15px;
        }

        QLabel { color: #FF3399; }

        QListWidget {
            background: rgba(255,255,255,0.08);
            border: 1px solid rgba(255,255,255,0.15);
            border-radius: 10px;
            padding: 8px;
            color: #FF3399; /* texto de items */
        }
        QListWidget::item {
            padding: 6px 8px;
        }
        QListWidget::item:selected {
            background: #FF3399;
            color: #111727;           /* texto oscuro para contraste */
            border-radius: 6px;
        }

        QPushButton {
            border-radius: 10px;
            padding: 10px 16px;
            font-weight: 500;
        }

        /* Primario: sólido rosa */
        #btnPrimary {
            background: #FF3399;
            color: #111727;
            border: 1px solid #FF3399;
        }
        #btnPrimary:hover   { background: #FF4AAD; border-color: #FF4AAD; }
        #btnPrimary:pressed { background: #E6007E; border-color: #E6007E; }
        #btnPrimary:disabled{ background: #80405F; border-color: #80405F; color: #1a1f2b; }

        /* Secundario: borde y texto rosa */
        #btnSecondary {
            background: transparent;
            color: #FF3399;
            border: 1px solid rgba(255,51,153,0.6);
        }
        #btnSecondary:hover   { border-color: #FF3399; background: rgba(255,51,153,0.10); }
        #btnSecondary:pressed { border-color: #FF3399; background: rgba(255,51,153,0.18); }
        #btnSecondary:disabled{ color: rgba(255,51,153,0.5); border-color: rgba(255,51,153,0.3); }
    )");
}


void Lobby::setupConnections() {
    connect(startButton, &QPushButton::clicked, this, &Lobby::onStartGame);
    connect(exitButton, &QPushButton::clicked, this, &Lobby::onExitLobby);
}


void Lobby::receive_snapshotlobby(const Snapshot_lobby& snapshot) {
    currentLobbyCode = snapshot.lobby_code;

    lobbyCodeWidget->setText("Código del lobby: " + QString::number(snapshot.lobby_code));
    playerList->clear();

    static const std::vector<QString> carNames = {
            "Familiar", "Sedán", "Deportivo", "Súper Deportivo", "Pick-Up", "GT", "Limusina"};

    for (const auto& player: snapshot.players_id) {

        QString carName = carNames[player.car_model];

        QString playerText =
                QString("%1 — %2").arg(QString::fromStdString(player.player_name)).arg(carName);

        playerList->addItem(playerText);
    }
}

void Lobby::listenServer() {
    while (keep_listening) {
        ServerEventReceiver event;
        if (!queue_receiver.try_pop(event)) {
            QThread::msleep(10);
            continue;
        }

        switch (event.type) {
            case ServerEventReceiverType::SNAPSHOT_LOBBY:
                QMetaObject::invokeMethod(
                        this, [this, event]() { receive_snapshotlobby(event.snapshot_lobby); });
                break;

            case ServerEventReceiverType::START_GAME:
                keep_listening = false;
                QMetaObject::invokeMethod(this, [this]() { accept(); });

                return;

            case ServerEventReceiverType::ERROR:
                QMetaObject::invokeMethod(this, [this]() { reject(); });
                return;

            default:
                break;
        }
    }
}


void Lobby::onStartGame() {
    ServerEventSender event;
    event.type = ServerEventSenderType::START_GAME;
    event.start_game.lobby_code = currentLobbyCode;

    queue_sender.try_push(event);
}


void Lobby::onExitLobby() {
    ServerEventSender event;
    event.type = ServerEventSenderType::LEAVE_LOBBY;
    queue_sender.try_push(event);
    reject();
}

Lobby::~Lobby() {
    keep_listening = false;
    if (listenerThread && listenerThread->isRunning()) {
        listenerThread->wait();
    }
}
