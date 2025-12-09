#include "JoinLobby.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>
#include <string>

#include <yaml-cpp/yaml.h>

#include "../../common/resource_paths.h"

JoinLobby::JoinLobby(QWidget* parent): QDialog(parent) {
    setWindowTitle("Unirse a partida");
    resize(900, 560);

    const std::string cfgPath = ResourcePaths::config() + "/config.yaml";
    StatBar::loadCarStatsFromConfig(QString::fromStdString(cfgPath));


    setupUI();
    setupStyle();

    updateCarPreview(carBox->currentText());
    statBar->update(carBox->currentIndex());
}

QString JoinLobby::carImagePathFor(const QString& name) const {
    static const QMap<QString, QString> mapping{
            {"Familiar", QString::fromStdString(ResourcePaths::assets() + "/CarImages/auto1.png")},
            {"Sedán", QString::fromStdString(ResourcePaths::assets() + "/CarImages/auto2.png")},
            {"Deportivo", QString::fromStdString(ResourcePaths::assets() + "/CarImages/auto3.png")},
            {"Súper Deportivo",
             QString::fromStdString(ResourcePaths::assets() + "/CarImages/auto4.png")},
            {"Pick-Up", QString::fromStdString(ResourcePaths::assets() + "/CarImages/auto5.png")},
            {"GT", QString::fromStdString(ResourcePaths::assets() + "/CarImages/auto6.png")},
            {"Limusina", QString::fromStdString(ResourcePaths::assets() + "/CarImages/auto7.png")},
    };

    return mapping.value(name, ":/cars/car.png");
}

// actualizamos imagen del auto si usuario cambia de auto
void JoinLobby::updateCarPreview(const QString& name) {
    const QString path = carImagePathFor(name);
    QPixmap px(path);

    const QSize target = carPreview->size();
    // escala la imagen al tamanio del cuadro
    QPixmap scaled = px.scaled(target, Qt::KeepAspectRatio, Qt::FastTransformation);
    carPreview->setPixmap(scaled);
}


QHBoxLayout* JoinLobby::createCarSelector() {
    auto* rowCar = new QHBoxLayout();

    // dividimos en columnas para en una mostrar la seleccion de autos
    // y en la otra la preview de las fotos

    // columna de la izquierda eleccion de autos
    auto* leftCol = new QVBoxLayout();
    carBox = new QComboBox(this);
    carBox->addItems(
            {"Familiar", "Sedán", "Deportivo", "Súper Deportivo", "Pick-Up", "GT", "Limusina"});
    leftCol->addWidget(carBox);
    leftCol->addStretch(1);

    // columna derecha (preview de las imagenes)
    auto* rightCol = new QVBoxLayout();
    auto* previewTitle = new QLabel("Vista previa", this);
    previewTitle->setStyleSheet("font-weight:600; color:#FF3399;");

    carPreview = new QLabel(this);
    carPreview->setFixedSize(220, 140);
    carPreview->setAlignment(Qt::AlignCenter);
    carPreview->setStyleSheet("background: rgba(255,255,255,0.06);"
                              "border: 1px solid rgba(255,255,255,0.14);"
                              "border-radius: 10px;");

    carPreview->setScaledContents(false);

    rightCol->addWidget(previewTitle);
    rightCol->addSpacing(8);
    rightCol->addWidget(carPreview);
    rightCol->addStretch(1);

    auto* statsTitle = new QLabel("Estadísticas", this);
    statsTitle->setStyleSheet("font-weight:600;");
    rightCol->addWidget(statsTitle);

    statBar = new StatBar(this);
    statBar->init(this);

    auto* statsWrapper = new QVBoxLayout();
    statBar->addToLayout(statsWrapper, 130);
    rightCol->addLayout(statsWrapper);

    rightCol->addStretch(1);


    // armar fila
    rowCar->addLayout(leftCol, 2);
    rowCar->addSpacing(24);
    rowCar->addLayout(rightCol, 1);

    return rowCar;
}


void JoinLobby::setupUI() {
    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(20, 20, 20, 20);
    root->setSpacing(12);

    auto* codeWidget = new QLabel("🔑 Código de la partida", this);
    codeWidget->setAlignment(Qt::AlignCenter);
    root->addWidget(codeWidget);

    // codigo unirse a otra lobby
    codeEdit = new QLineEdit(this);
    // solo se pueden poner numeros de 0 a 999999
    codeEdit->setValidator(new QIntValidator(0, 999999, this));
    root->addWidget(codeEdit);

    root->addSpacing(12);

    //  Nombre
    auto* nameWidget = new QLabel("👤 Tu nombre", this);
    nameWidget->setAlignment(Qt::AlignCenter);
    root->addWidget(nameWidget);

    nameEdit = new QLineEdit(this);
    nameEdit->setMaxLength(10);
    root->addWidget(nameEdit);

    //  Seleccion de auto + preview
    auto* carSelectedWidget = new QLabel("Elegí tu auto:", this);
    carSelectedWidget->setAlignment(Qt::AlignCenter);
    root->addWidget(carSelectedWidget);

    root->addLayout(createCarSelector());

    //  Botones
    auto* btnRow = new QHBoxLayout();
    auto* cancel = new QPushButton("Cancelar", this);
    auto* ok = new QPushButton("Aceptar", this);

    cancel->setObjectName("btnSecondary");
    ok->setObjectName("btnPrimary");

    btnRow->addStretch();
    btnRow->addWidget(cancel);
    btnRow->addWidget(ok);
    root->addLayout(btnRow);

    setupConnections(ok, cancel);
}

void JoinLobby::setupStyle() {
    setStyleSheet(R"(
        QDialog {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #111727, stop:1 #0a0f1d);
            font-family: "Poppins", "Segoe UI", sans-serif;
            font-size: 14px;
            color: #FF3399; /* texto global rosa */
        }

        QLabel {
            font-weight: 500;
            color: #FF3399; /* labels rosas */
        }

        QLineEdit, QComboBox {
            background: rgba(255,255,255,0.10);
            border: 1px solid rgba(255,255,255,0.20);
            border-radius: 8px;
            padding: 8px 10px;
            color: #FF3399; /* texto rosa */
        }
        QLineEdit:focus, QComboBox:focus {
            border-color: #FF3399;
        }

        /* Lista desplegable del combo */
        QComboBox QAbstractItemView {
            background: #1b2333;
            color: #FF3399;
            border: 1px solid #3c4b6a;
            outline: none;
            padding: 4px 0;
            selection-background-color: #FF3399;
            selection-color: #111727;
        }
        QComboBox QAbstractItemView::item {
            min-height: 28px;
            padding: 6px 12px;
        }

        /* Botones base */
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

        /* Secundario: borde rosa */
        #btnSecondary {
            background: transparent;
            color: #FF3399;
            border: 1px solid rgba(255, 51, 153, 0.6);
        }
        #btnSecondary:hover   { border-color: #FF3399; background: rgba(255, 51, 153, 0.10); }
        #btnSecondary:pressed { border-color: #FF3399; background: rgba(255, 51, 153, 0.18); }
        #btnSecondary:disabled{ color: rgba(255, 51, 153, 0.5); border-color: rgba(255, 51, 153, 0.3); }
    )");
}

// valida nombre puesto por usuario
bool JoinLobby::validateName() {
    const QString explicitName = nameEdit->text().trimmed();
    const bool accepted = (explicitName.length() >= 2 && explicitName.length() <= 10);

    return accepted;
}


// valida codigo puesto por usuario
bool JoinLobby::validateCode() {
    const QString explicitCode = codeEdit->text().trimmed();
    const bool accepted = !explicitCode.isEmpty();

    return accepted;
}


void JoinLobby::setupConnections(QPushButton* ok, QPushButton* cancel) {
    // validar nombre
    connect(nameEdit, &QLineEdit::textChanged, this, [this](const QString&) { validateName(); });

    // validar código
    connect(codeEdit, &QLineEdit::textChanged, this, [this](const QString&) { validateCode(); });

    // botón cancelar
    connect(cancel, &QPushButton::clicked, this, &QDialog::reject);

    // botón aceptar
    connect(ok, &QPushButton::clicked, this, [this]() {
        const bool nameOk = validateName();
        const bool codeOk = validateCode();

        if (!nameOk || !codeOk)
            return;

        qDebug() << "Player name:" << nameEdit->text().trimmed();
        accept();
    });

    connect(carBox, &QComboBox::currentIndexChanged, this, [this](int idx) {
        updateCarPreview(carBox->itemText(idx));
        statBar->update(idx);
    });

    // cambio de auto → actualizar preview
    connect(carBox, &QComboBox::currentTextChanged, this, &JoinLobby::updateCarPreview);
}


JoinLobby::~JoinLobby() = default;

QString JoinLobby::code() const { return codeEdit->text(); }

int JoinLobby::getCarSelected() const { return carBox->currentIndex(); }

std::string JoinLobby::getNameSelected() const { return nameEdit->text().trimmed().toStdString(); }
