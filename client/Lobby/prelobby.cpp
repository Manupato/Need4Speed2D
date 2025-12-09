#include "prelobby.h"

#include <QEvent>
#include <QLabel>
#include <QMessageBox>
#include <QPixmap>
#include <QStackedLayout>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "../../common/resource_paths.h"

#include "CreateLobby.h"
#include "JoinLobby.h"
#include "Lobby.h"

PreLobby::PreLobby(Queue<ServerEventSender>& queue_sender,
                   Queue<ServerEventReceiver>& queue_receiver, GameSoundManager& sound_manager,
                   QWidget* parent):
        QDialog(parent),
        queue_sender(queue_sender),
        queue_receiver(queue_receiver),
        sound_manager(sound_manager) {
    setWindowTitle("Need for Speed 2D — Pre-Partida");
    setModal(true);  // impide que el usuario pueda tocar otras ventanas
    resize(690, 630);

    setupUI();
}

Action PreLobby::resultAction() {
    Action act = action_;
    action_ = Action::None;
    return act;
}


std::vector<std::string> CreateLobby::selectedMapFilesStd() const {
    std::vector<std::string> out;
    const QStringList q = selectedMapFiles();  // ya trae ".json"
    out.reserve(q.size());
    std::transform(q.begin(), q.end(), std::back_inserter(out),
                   [](const QString& s) { return s.toStdString(); });
    return out;
}

void PreLobby::sendCreateLobbyEvent(const LobbyConfig& config,
                                    const std::vector<std::string>& mapas,
                                    const std::string& name) {
    if (mapas.empty()) {
        QMessageBox::warning(this, tr("Faltan mapas"),
                             tr("Tenés que elegir al menos un mapa antes de crear la partida."));
        return;
    }

    ServerEventSender event;
    event.type = ServerEventSenderType::CREATE_LOBBY;

    const uint8_t modelo = static_cast<uint8_t>(config.carId);
    event.create_lobby = {modelo, name, mapas};

    queue_sender.try_push(event);
    startListeningServer();
}


// que pasa cuando se apretan los botones de crear lobby y unirse a lobby
void PreLobby::setupConnections(QPushButton* btnCrear, QPushButton* btnUnirse) {
    // cuando se toca el el boton crear que haga esto
    connect(btnCrear, &QPushButton::clicked, this, [this] {
        CreateLobby lobby(this);
        if (lobby.exec() == QDialog::Accepted) {
            usuario_config = lobby.resultConfig();
            action_ = Action::Create;

            std::vector<std::string> map_names = lobby.selectedMapFilesStd();

            std::string name = lobby.getNameSelected();

            sendCreateLobbyEvent(usuario_config, map_names, name);
        }
    });

    connect(btnUnirse, &QPushButton::clicked, this, [this] {
        JoinLobby join_lobby(this);
        if (join_lobby.exec() == QDialog::Accepted) {

            action_ = Action::Join;

            uint32_t codigo = join_lobby.code().toUInt();

            ServerEventSender event;
            event.type = ServerEventSenderType::JOIN_LOBBY;

            const uint8_t modelo = join_lobby.getCarSelected();
            const std::string name = join_lobby.getNameSelected();
            event.join_lobby = {modelo, codigo, name};

            queue_sender.try_push(event);
            startListeningServer();
        }
    });
}

void PreLobby::setupStyle() {
    setStyleSheet(R"(
        QDialog {
            font-family: "Poppins", "Segoe UI", sans-serif;
            font-size: 15px;
            color: #FF3399;
        }

        #prelobbyCard {
            background: rgba(255, 255, 255, 0.05);
            border: 1px solid rgba(255, 255, 255, 0.15);
            border-radius: 18px;
        }

        #prelobbyTitle {
            font-size: 26px;
            font-weight: 600;
            color: #FF3399;
        }

        QPushButton {
            border-radius: 10px;
            padding: 12px 18px;
            font-weight: 500;
        }

        #btnPrimary {
            background: #FF3399;
            color: #111727;
            border: 1px solid #FF3399;
        }
        #btnPrimary:hover   { background: #FF4AAD; border-color: #FF4AAD; }
        #btnPrimary:pressed { background: #E6007E; border-color: #E6007E; }

        #btnSecondary {
            background: #111727;
            color: #FF3399;
            border: 1px solid #FF3399;
        }
        #btnSecondary:hover   { background: #1a2238; }
        #btnSecondary:pressed { background: #0d1324; }
    )");
}

void PreLobby::setupVolumeControls(QVBoxLayout* root) {

    auto* volumeLayout = new QHBoxLayout();

    auto* btnVolDown = new QPushButton("Bajar volumen", this);
    auto* btnVolUp = new QPushButton("Subir volumen", this);

    // Reutilizamos el estilo #btnSecondary
    btnVolDown->setObjectName("btnSecondary");
    btnVolUp->setObjectName("btnSecondary");

    btnVolDown->setFixedWidth(150);
    btnVolUp->setFixedWidth(150);


    volumeLayout->addWidget(btnVolDown);
    volumeLayout->addWidget(btnVolUp);
    volumeLayout->addStretch();

    // Los pongo al final del layout principal (abajo)
    root->addLayout(volumeLayout);

    // bajar el volumen
    connect(btnVolDown, &QPushButton::clicked, this,
            [this] { sound_manager.audio_configuration(MusicConfigType::DECREASE); });

    //  subir volumen
    connect(btnVolUp, &QPushButton::clicked, this,
            [this] { sound_manager.audio_configuration(MusicConfigType::INCREASE); });
}


void PreLobby::setupUI() {

    bgLarge = QPixmap(
            QString::fromStdString(ResourcePaths::assets() + "/Lobby_images/prelobby_bg_max.jpg"));

    smallerImage = QPixmap(
            QString::fromStdString(ResourcePaths::assets() + "/Lobby_images/prelobby_bg.jpg"));

    // Qlabel es un widget solo para poner imagenes
    wallpaper = new QLabel(this);
    // hace que no se puedan seleccionar otras penstanias cuando estas en esta
    wallpaper->setScaledContents(true);
    // baja el widget a la ultima prioridad de la pila de widgets
    wallpaper->lower();
    wallpaper->setPixmap(QPixmap(
            QString::fromStdString(ResourcePaths::assets() + "/Lobby_images/prelobby_bg.jpg")));

    // root administra los widgets en forma vertical
    auto* root = new QVBoxLayout(this);


    // base de widget que tiene borde
    auto* card = new QFrame(this);
    card->setObjectName("prelobbyCard");
    card->setMinimumSize(600, 300);

    // widgetOrganaizer no es un widget es un organizador de widgets
    auto* widgetOrganaizer = new QVBoxLayout(card);
    widgetOrganaizer->setSpacing(16);

    // widget del titulo que va dentro de card
    auto* title = new QLabel("Elegí una opción", card);
    title->setObjectName("prelobbyTitle");

    // widget de botones de tipo card
    auto* btnCrear = new QPushButton("Crear partida", card);
    auto* btnUnirse = new QPushButton("Unirse a partida", card);

    btnCrear->setObjectName("btnPrimary");
    btnUnirse->setObjectName("btnSecondary");

    // al organizador de widgets le vamos metiendo uno por uno el widget que queremos
    widgetOrganaizer->addWidget(title, 0, Qt::AlignHCenter);
    widgetOrganaizer->addStretch(1);
    widgetOrganaizer->addWidget(btnCrear);
    widgetOrganaizer->addWidget(btnUnirse);
    widgetOrganaizer->addStretch(1);

    // lo unico que hace root es centrar la card que llenamos de cosas anteriormente
    root->addStretch();
    root->addWidget(card, 0, Qt::AlignCenter);  // la centra en ambos ejes
    root->addStretch();

    setupVolumeControls(root);

    setupStyle();
    setupConnections(btnCrear, btnUnirse);
}


void PreLobby::resizeEvent(QResizeEvent* event) {
    QDialog::resizeEvent(event);

    if (wallpaper) {
        wallpaper->setGeometry(this->rect());
    }
}

void PreLobby::start_music() { sound_manager.playLobbyMusic(); }


void PreLobby::changeEvent(QEvent* event) {
    if (event->type() == QEvent::WindowStateChange) {

        if (isMaximized())
            wallpaper->setPixmap(bgLarge);
        else
            wallpaper->setPixmap(smallerImage);

        wallpaper->setGeometry(this->rect());
    }

    QDialog::changeEvent(event);
}


void PreLobby::handleServerEvent(const ServerEventReceiver& event) {
    switch (event.type) {

        case ServerEventReceiverType::SUCESS: {

            Lobby lobby(queue_sender, queue_receiver,
                        this);  // si es aceptado por server creamos lobby para que se puedan unir

            int result = lobby.exec();

            if (result == QDialog::Accepted) {
                sound_manager.stop_music();
                accept();
            } else {
                action_ = Action::None;  // por si el usuario despues cierra todo
            }

            break;
        }

        case ServerEventReceiverType::ERROR:
            QMessageBox::warning(this, ("Error"),
                                 ("No se pudo crear/unir a la partida. Intentá de nuevo."));
            break;

        default:
            break;
    }
}

void PreLobby::startListeningServer() {
    ServerEventReceiver event = queue_receiver.pop();
    handleServerEvent(event);
}
