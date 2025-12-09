#ifndef CREATE_LOBBY_H
#define CREATE_LOBBY_H

#include <QDialog>
#include <QLineEdit>
#include <QStringList>
#include <string>
#include <vector>

#include "StatBar.h"

class QComboBox;
class QLabel;
class QFrame;
class QComboBox;
class QFormLayout;
class QHBoxLayout;
class QLineEdit;
class QVBoxLayout;

struct LobbyConfig {
    int carId;
    int mapId;
};

class CreateLobby: public QDialog {
public:
    explicit CreateLobby(QWidget* parent = nullptr);

    QString getCarSelected() const;

    QString getMapSelected() const;

    LobbyConfig resultConfig() const;

    std::vector<std::string> selectedMapFilesStd() const;

    std::string getNameSelected() const;

    QString playerName() const { return playerNameEdit ? playerNameEdit->text() : ""; }

protected:
    bool eventFilter(QObject* obj, QEvent* ev) override;

private:
    QComboBox* carBox = nullptr;
    QComboBox* mapBox = nullptr;  // alias a mapsCombo

    bool handleMapSpecificClick(QEvent* ev);

    bool handleComboClick(QEvent* ev);
    QFrame* createCard(QWidget* parent);
    QLabel* title = nullptr;
    void createTitle();
    QFrame* card = nullptr;
    QFormLayout* createContentAndSelectionBoxes(QWidget* parent);
    QStringList jsonToNames(const QString& folder) const;
    void setupStyle();
    void setupUI(QPushButton*& startBtn, QPushButton*& cancelBtn);
    QVBoxLayout* createPreviewColumn(QWidget* parent);
    void setupConnections(QPushButton* startBtn, QPushButton* cancelBtn);
    bool validatePlayerName();
    void setupMapsCombo();
    QLabel* carPreview = nullptr;
    QString carImagePathFor(const QString& name) const;
    void updateCarPreview(const QString& name);

    QComboBox* mapsCombo = nullptr;

    QStringList selectedMapFiles() const;

    void updateMapsSummaryText();

    QLineEdit* playerNameEdit = nullptr;

    StatBar* statBar = nullptr;
};

#endif
