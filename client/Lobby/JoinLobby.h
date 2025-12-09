#ifndef JOIN_LOBBY_H
#define JOIN_LOBBY_H

#include <QComboBox>
#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <string>

#include "StatBar.h"

class QLineEdit;
class QComboBox;
class QPushButton;
class QHBoxLayout;

class JoinLobby: public QDialog {
public:
    explicit JoinLobby(QWidget* parent = nullptr);

    ~JoinLobby() override;

    QString code() const;

    int getCarSelected() const;

    std::string getNameSelected() const;

private:
    bool validateName();

    bool validateCode();

    QHBoxLayout* createCarSelector();

    QLineEdit* codeEdit{nullptr};

    QLineEdit* nameEdit{nullptr};

    QComboBox* carBox{nullptr};

    QLabel* carPreview;

    StatBar* statBar = nullptr;

    void setupUI();

    void setupConnections(QPushButton* ok, QPushButton* cancel);

    void setupStyle();

    QString carImagePathFor(const QString& name) const;

    void updateCarPreview(const QString& name);
};

#endif
