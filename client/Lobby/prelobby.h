#ifndef TALLER_DE_PROGRAMACION_TP_GRUPAL_2025C2_GRUPO_2_PRELOBBY_H
#define TALLER_DE_PROGRAMACION_TP_GRUPAL_2025C2_GRUPO_2_PRELOBBY_H

#include <QDialog>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <string>
#include <vector>

#include <qstackedwidget.h>

#include "../../common/queue.h"
#include "../Game_GUI/GameSoundManager.h"
#include "../Game_GUI/sound/Sound.h"
#include "../ServerEvent.h"

#include "CreateLobby.h"

class GameSoundManager;
enum Action { None, Create, Join };
class PreLobby: public QDialog {

private:
    Action action_ = Action::None;

    LobbyConfig usuario_config{};

    Queue<ServerEventSender>& queue_sender;

    Queue<ServerEventReceiver>& queue_receiver;

    QLabel* wallpaper = nullptr;

    QPixmap smallerImage;

    QPixmap bgLarge;

    GameSoundManager& sound_manager;

    void setupUI();

    void handleServerEvent(const ServerEventReceiver& event);

    void startListeningServer();

    void setupStyle();

    void setupConnections(QPushButton* btnCrear, QPushButton* btnUnirse);

    void changeEvent(QEvent* event) override;

    void sendCreateLobbyEvent(const struct LobbyConfig& config,
                              const std::vector<std::string>& mapas, const std::string& name);

    void setupVolumeControls(QVBoxLayout* root);

public:
    explicit PreLobby(Queue<ServerEventSender>& queue_sender,
                      Queue<ServerEventReceiver>& queue_receiver, GameSoundManager& sound_manager,
                      QWidget* parent = nullptr);

    Action resultAction();

    LobbyConfig config() const { return usuario_config; }

    void resizeEvent(QResizeEvent*) override;

    void start_music();
};


#endif  // TALLER_DE_PROGRAMACION_TP_GRUPAL_2025C2_GRUPO_2_PRELOBBY_H
