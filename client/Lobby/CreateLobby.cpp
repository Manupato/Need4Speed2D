#include "CreateLobby.h"

#include <QAbstractItemView>
#include <QComboBox>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QEvent>
#include <QFileInfo>
#include <QFont>
#include <QFormLayout>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QModelIndex>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QStandardItemModel>
#include <QTimer>
#include <QVBoxLayout>
#include <string>

#include "../../common/resource_paths.h"


CreateLobby::CreateLobby(QWidget* parent): QDialog(parent) {
    setWindowTitle("Configuración de la partida");
    resize(900, 560);  // ventana estilo launcher

    const std::string cfgPath = ResourcePaths::config() + "/config.yaml";
    StatBar::loadCarStatsFromConfig(QString::fromStdString(cfgPath));

    QPushButton* startBtn = nullptr;
    QPushButton* cancelBtn = nullptr;

    setupUI(startBtn, cancelBtn);
    QTimer::singleShot(0, this, [this] {
        updateCarPreview(carBox->currentText());
        statBar->update(carBox->currentIndex());
    });
    setupStyle();

    // chequeamos si el nombre del jugador es valido o no y dependiendo de eso lo conectamos o no
    setupConnections(startBtn, cancelBtn);


    carBox->setCurrentIndex(0);
    mapBox->setCurrentIndex(-1);
}

void CreateLobby::setupConnections(QPushButton* startBtn, QPushButton* cancelBtn) {
    // botón "Comenzar partida"
    connect(startBtn, &QPushButton::clicked, this, [this] {
        if (!validatePlayerName())
            return;  // si el nombre no es válido, seguimos

        accept();
    });

    connect(carBox, &QComboBox::currentIndexChanged, this, [this](int idx) {
        updateCarPreview(carBox->itemText(idx));
        statBar->update(idx);
    });

    // botón "Cancelar"
    connect(cancelBtn, &QPushButton::clicked, this, [this] { reject(); });

    // cambio de auto → actualizar preview
    connect(carBox, &QComboBox::currentTextChanged, this, &CreateLobby::updateCarPreview);
}


bool CreateLobby::validatePlayerName() {
    QString nameSelected = playerNameEdit->text().trimmed();
    bool valid = (nameSelected.length() >= 2 && nameSelected.length() <= 10);

    if (valid) {
        playerNameEdit->setStyleSheet("");
    }

    return valid;
}


void CreateLobby::setupUI(QPushButton*& startBtn, QPushButton*& cancelBtn) {
    auto* root = new QVBoxLayout(this);
    root->setSpacing(0);

    card = createCard(this);
    createTitle();

    auto* sorter_widgets = new QVBoxLayout(card);
    sorter_widgets->setSpacing(18);
    sorter_widgets->addWidget(title);
    sorter_widgets->addSpacing(8);

    // Columna izquierda (nombre, auto, mapas)
    auto* selectedAttributes = createContentAndSelectionBoxes(card);

    auto* topRow = new QHBoxLayout();
    topRow->addLayout(selectedAttributes, 2);

    // Columna derecha (preview + stat bars)
    auto* rightCol = createPreviewColumn(card);
    topRow->addSpacing(24);
    topRow->addLayout(rightCol, 1);

    sorter_widgets->addLayout(topRow);

    // Botones
    auto* btnRow = new QHBoxLayout();
    startBtn = new QPushButton("Comenzar partida", card);
    cancelBtn = new QPushButton("Cancelar", card);
    startBtn->setObjectName("btnPrimary");

    btnRow->addStretch(1);
    btnRow->addWidget(cancelBtn);
    btnRow->addSpacing(12);
    btnRow->addWidget(startBtn);

    sorter_widgets->addSpacing(8);
    sorter_widgets->addLayout(btnRow);

    auto* center = new QHBoxLayout();
    center->addStretch(1);
    center->addWidget(card);
    center->addStretch(1);

    root->addStretch(1);
    root->addLayout(center);
    root->addStretch(1);
}

// se crea la columna de la preview de las imagenes
QVBoxLayout* CreateLobby::createPreviewColumn(QWidget* parent) {
    // se inicializa el widget
    auto* previewCol = new QVBoxLayout();

    // título "Vista previa"
    auto* previewTitle = new QLabel("Vista previa", parent);
    previewTitle->setStyleSheet("font-weight:600;");

    // imagenes del auto elegido
    carPreview = new QLabel(parent);
    carPreview->setFixedSize(220, 140);
    carPreview->setAlignment(Qt::AlignCenter);
    carPreview->setStyleSheet("background: rgba(255,255,255,0.06);"
                              "border: 1px solid rgba(255,255,255,0.14);"
                              "border-radius: 10px;");
    carPreview->setScaledContents(false);
    updateCarPreview(carBox->currentText());

    // agrego a la columna de preview de imagenes cada widget
    previewCol->addWidget(previewTitle);
    previewCol->addSpacing(8);
    previewCol->addWidget(carPreview);
    previewCol->addStretch(1);

    auto* statsTitle = new QLabel("Estadísticas", parent);
    statsTitle->setStyleSheet("font-weight:600;");
    previewCol->addSpacing(12);
    previewCol->addWidget(statsTitle);

    statBar = new StatBar(parent);
    statBar->init(parent);

    auto* statsLayout = new QVBoxLayout();
    statBar->addToLayout(statsLayout, 150);

    previewCol->addLayout(statsLayout);

    return previewCol;
}

std::string CreateLobby::getNameSelected() const {
    return playerNameEdit->text().trimmed().toStdString();
}

bool CreateLobby::handleMapSpecificClick(QEvent* event) {
    // solo me importa el evento si lo dejo de precionar
    if (event->type() != QEvent::MouseButtonRelease)
        return false;

    // como ya se que es un evento de mouse  lo casteo para poder usarlo
    // cada widget tiene un viewport(los clicks que se hacen) y un model(los intems que tiene
    // realmente el widget
    auto* event_mouse = static_cast<QMouseEvent*>(event);
    auto* mapas = mapsCombo->view();

    // me tradice en que mapa especifico se cliqueo
    const QModelIndex map_ubication = mapas->indexAt(event_mouse->pos());

    if (!map_ubication.isValid())
        return true;

    // extrae los modelos que tiene el widget, los modelos son los intemos que guarda el widget
    auto* items = qobject_cast<QStandardItemModel*>(mapsCombo->model());
    if (!items)
        return true;

    // me extrae realemente el mapa que cliqueo de la ubicacion del clickeo anterior
    QStandardItem* mapa_real = items->itemFromIndex(map_ubication);
    if (!mapa_real)
        return true;

    // aca es realmente donde si no estaba cliqueado lo cliquea y al reves, basicamente un toggle
    mapa_real->setCheckState(mapa_real->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);

    updateMapsSummaryText();
    return true;
}

bool CreateLobby::handleComboClick(QEvent* ev) {
    if (ev->type() != QEvent::MouseButtonRelease)
        return false;

    // se fija que si esta cerrada la pestania de los mapas, si esta la abre
    if (!mapsCombo->view()->isVisible())
        mapsCombo->showPopup();

    return true;
}

// cada vez que se llame a installEventFilter va a venir aca
//  el object puede ser o un mapcombo o viewport etc y el event puede ser un click de un mouse etc
bool CreateLobby::eventFilter(QObject* actual_object, QEvent* event) {

    if (!mapsCombo)
        return QDialog::eventFilter(actual_object, event);

    // 1) Click en los items del popup (vista interna)
    if (actual_object == mapsCombo->view()->viewport())
        return handleMapSpecificClick(event);

    // clickeo en un mapa especifico o en el texto que sale antes de tocar un mapa?
    if (actual_object == mapsCombo || actual_object == mapsCombo->lineEdit())
        return handleComboClick(event);

    // Si no lo procesamos, seguir con el comportamiento normal
    return QDialog::eventFilter(actual_object, event);
}


QStringList CreateLobby::selectedMapFiles() const {
    // creamos lista vacia a devolver
    QStringList mapSelectedList;
    if (!mapsCombo)
        return mapSelectedList;

    // extraemos el modelo del combo, modelo: los items que guarda el widget y los casteamos para
    // poder usarlos
    auto* iteams = qobject_cast<QStandardItemModel*>(mapsCombo->model());
    if (!iteams)
        return mapSelectedList;

    // chequeamos si cada mapa seleccionado existe o se puede togglear
    for (int i = 0; i < iteams->rowCount(); ++i) {
        QStandardItem* actualMap = iteams->item(i);


        if (!actualMap || !(actualMap->flags() & Qt::ItemIsUserCheckable))
            continue;

        // si el mapa esta cliqueado lo agregamos a la lista con el .json
        if (actualMap->checkState() == Qt::Checked) {
            mapSelectedList << (actualMap->text() + ".json");
        }
    }
    return mapSelectedList;
}


void CreateLobby::updateMapsSummaryText() {
    if (!mapsCombo)
        return;
    // mapas que actualmente se seleccionaron
    const QStringList files = selectedMapFiles();

    // si no se selecciono un mapa borra los mapas que haya y se vuelve a poner el placeholder
    if (files.isEmpty()) {

        mapsCombo->setEditText(QString());
        if (auto* mapText = mapsCombo->lineEdit()) {
            mapText->setPlaceholderText("Elegí uno o más mapas…");
        }
        mapsCombo->setCurrentIndex(-1);
        return;
    }

    // extraigo .json de cada mapa selecionado
    QStringList mapList;
    mapList.reserve(files.size());

    for (const QString& file: files) {
        mapList << QFileInfo(file).baseName();
    }

    // si hay menos de 3 mapas mostralos
    if (mapList.size() <= 3) {
        mapsCombo->lineEdit()->setText(mapList.join(", "));
    } else {
        mapsCombo->lineEdit()->setText(QString("%1 mapas seleccionados").arg(mapList.size()));
    }
}


QString CreateLobby::carImagePathFor(const QString& name) const {
    static const QHash<QString, QString> mapping{
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

// cargar imagen de auto pasada
void CreateLobby::updateCarPreview(const QString& name) {
    // busca la ruta
    const QString path = carImagePathFor(name);
    QPixmap px(path);

    // escala imagen y la pone en el label
    const QSize size = carPreview->size();
    QPixmap scaled = px.scaled(size, Qt::KeepAspectRatio, Qt::FastTransformation);
    carPreview->setPixmap(scaled);
}

// crea widget para mostrar el nombre de "configurar partida
void CreateLobby::createTitle() {
    title = new QLabel("Configurar partida",
                       card);            // crea cuadrado de tipo la carta con el nuevo texto
    title->setObjectName("lobbyTitle");  // le cambia el nombre al objecto para despues en CSS
}

void CreateLobby::setupMapsCombo() {
    // 3) Cargar nombres de mapas (sin .json)
    const QString folder = QString::fromStdString(ResourcePaths::userMaps());
    const QStringList nombresMapas = jsonToNames(folder);

    // inicializa un qline interno(un widget para ingresar texto), es lo que se muestra antes de
    // elegir algo
    mapsCombo->setEditable(true);
    // hace que el usuario no pueda ingresar texto ahi
    mapsCombo->lineEdit()->setReadOnly(true);  // Placeholder: cada ves que no hay nada
                                               // seleccionado"
    mapsCombo->lineEdit()->setPlaceholderText("Elegí uno o más mapas…");

    // cada vez que pase un evento se llame a eventfilter
    mapsCombo->installEventFilter(this);
    mapsCombo->lineEdit()->installEventFilter(this);

    auto* items = qobject_cast<QStandardItemModel*>(mapsCombo->model());
    // la view es lo que realemnte se muestra
    auto* view = mapsCombo->view();
    view->setSelectionMode(QAbstractItemView::NoSelection);

    // cada vez que se preciona en algun lugar llama al evenFilter
    view->viewport()->installEventFilter(this);

    if (nombresMapas.isEmpty()) {
        mapsCombo->addItem("<< No hay mapas >>");
        mapsCombo->setInsertPolicy(QComboBox::NoInsert);
        mapsCombo->setCurrentIndex(-1);  // clave: sin selección inicial
        mapsCombo->setEditText(QString());
        if (auto* it = items->item(0))
            it->setEnabled(false);
    } else {
        // se agrega cada mapa que encontro de la lista de los nombres
        for (const QString& nombre: nombresMapas) {
            mapsCombo->addItem(nombre);

            QStandardItem* mapaEspecifico = items->item(mapsCombo->count() - 1);
            // setea las flags especificas
            mapaEspecifico->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable |
                                     Qt::ItemIsSelectable);
            mapaEspecifico->setData(Qt::Unchecked, Qt::CheckStateRole);
        }
    }


    // al principio, actualiza el texto
    updateMapsSummaryText();
}

QFormLayout* CreateLobby::createContentAndSelectionBoxes(QWidget* parent) {
    // creamos el widget donde estara una lista seleccionable
    carBox = new QComboBox(parent);
    // lo mismo que carbox pero para los mapas seleccionables
    mapsCombo = new QComboBox(parent);

    // aniadimos a la lista cada nombre de auto especifico
    carBox->addItems(
            {"Familiar", "Sedán", "Deportivo", "Súper Deportivo", "Pick-Up", "GT", "Limusina"});

    // configuramos todo lo relacionado a la lista de mapas (modelo, eventos, etc.)
    setupMapsCombo();

    auto* selectedAttributes = new QFormLayout();

    mapBox = mapsCombo;

    // se crea el widget del nombre
    playerNameEdit = new QLineEdit(parent);
    playerNameEdit->setPlaceholderText("Ingresá tu nombre…");
    playerNameEdit->setMaxLength(10);
    playerNameEdit->setMinimumHeight(40);
    playerNameEdit->setObjectName("playerNameEdit");

    // alinear elementos
    selectedAttributes->addRow(new QLabel("Nombre:", parent), playerNameEdit);
    selectedAttributes->addRow(new QLabel("Tipo de auto:", parent), carBox);
    selectedAttributes->addRow(new QLabel("Mapas:", parent), mapsCombo);

    return selectedAttributes;
}


// extraemos los nombres de los .json que nos da el editor para
// mostrarlos al usuario
QStringList CreateLobby::jsonToNames(const QString& folder) const {
    QDir dirrectorio_mapas(folder);  // puntero a ubicacion para usarla despues

    const QStringList nameFiles = dirrectorio_mapas.entryList(
            QStringList{"*.json"}, QDir::Files);  // devulve string con los nombres de los archivos
                                                  // adentro de la carpeta que terminen con .json
    QStringList namesWithoutJson;                 // lista donde van nombres sin .json
    namesWithoutJson.reserve(nameFiles.size());

    // se tirera la lista de nombres con .json y se le extrae a esa string el .json
    for (const QString& fileName: nameFiles) {
        if (fileName.endsWith(".json", Qt::CaseInsensitive))
            namesWithoutJson << fileName.left(fileName.length() - 5);  // sin ".json"
    }
    return namesWithoutJson;
}


// crea el contendedor visual de la lobby donde van a ir mas cosas en un futuro
QFrame* CreateLobby::createCard(QWidget* parent) {
    card = new QFrame(parent);  // crear cuadrado principal de lobby
    card->setObjectName("lobbyCard");
    card->setMinimumSize(720, 420);
    card->setMaximumWidth(760);
    return card;
}

void CreateLobby::setupStyle() {
    setStyleSheet(R"(
        QDialog {
            background: qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #111727, stop:1 #0a0f1d);
            font-family: "Poppins", "Segoe UI", sans-serif;
            font-size: 15px;
            color: #FF3399; /* texto global rosa */
        }

        #lobbyCard {
            background: rgba(255,255,255,0.06);
            border: 1px solid rgba(255,255,255,0.14);
            border-radius: 18px;
        }

        #lobbyTitle {
            font-size: 24px;
            font-weight: 600;
            color: #FF3399; /* título rosa */
            letter-spacing: 0.3px;
        }

        QLabel { color: #FF3399; } /* labels rosa */

        QComboBox {
            background: rgba(255,255,255,0.10);
            border: 1px solid rgba(255,255,255,0.20);
            border-radius: 10px;
            padding: 8px 12px;
            color: #FF3399; /* texto del combo rosa */
        }
        QComboBox:hover { border-color: #4f8cff; }
        QComboBox::drop-down { border: none; width: 28px; }

        QPushButton {
            background: rgba(255,255,255,0.10);
            border: 1px solid rgba(255,255,255,0.20);
            border-radius: 10px;
            padding: 10px 16px;
            color: #FF3399; /* texto de botones rosa */
        }
        QPushButton:hover  { background: rgba(255,255,255,0.16); border-color: #5b97ff; }
        QPushButton:pressed{ background: #2a6ae6; border-color: #2a6ae6; color: white; }
        QPushButton:focus  { border-color: #6aa8ff; }

        #btnPrimary {
            background:#FF3399;
            color:#111727;
            border: none;
        }
        #btnPrimary:hover  { background: #5b97ff; }
        #btnPrimary:pressed{ background: #3a7aff; }

        /* Popup del combo (lista) */
        QComboBox QAbstractItemView {
            background: #1b2333;
            color: #FF3399;              /* texto de items rosa */
            border: 1px solid #3c4b6a;
            outline: none;
            padding: 4px 0;
            selection-background-color: #4f8cff;
            selection-color: white;      /* puede ser blanco para legibilidad */
        }
        QComboBox QAbstractItemView::item {
            min-height: 28px;
            padding: 6px 12px;
        }

        #playerNameEdit {
            background: rgba(255,255,255,0.10);
            border: 1px solid rgba(255,255,255,0.20);
            border-radius: 10px;
            padding: 8px 12px;
            color: #FF3399;
        }
        #playerNameEdit:focus {
            border-color: #6aa8ff;
        }

        /* Flechita del combo en rosa */
        QComboBox::down-arrow {
            image: none;
            width: 0; height: 0;
            border-left: 6px solid transparent;
            border-right: 6px solid transparent;
            border-top: 8px solid #FF3399;
            margin-right: 10px;
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 28px;
            border: none;
        }
        QComboBox:focus { border: 1px solid #6aa8ff; }
    )");
}


LobbyConfig CreateLobby::resultConfig() const {
    return {carBox->currentIndex(), mapBox->currentIndex()};
}

QString CreateLobby::getCarSelected() const { return carBox->currentText(); }
QString CreateLobby::getMapSelected() const { return mapBox->currentText(); }
