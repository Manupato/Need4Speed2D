#include "mainwindow.h"

#include "../common/resource_paths.h"

MainWindow::MainWindow(QWidget* parent): QMainWindow(parent) {
    setCentralWidget(&mapView);
    setupPaths();
    setupMenus();
    setupActions();
    connectActions();
    createToolsToolbar();

    statusBar()->showMessage("Modo: Ninguno");
    resize(1280, 720);
    setWindowTitle("Editor de mapas - Need for Speed 2D");
}

void MainWindow::setupPaths() {
    QString assetsPrefix = QString::fromStdString(ResourcePaths::assets());
    QString userMapsPrefix = QString::fromStdString(ResourcePaths::userMaps());

    protectedMaps = {"walkable_tiles_sanandreas_completo.json",
                     "walkable_tiles_libertycity_completo.json",
                     "walkable_tiles_vicecity_completo.json"};

    baseMaps = QDir::cleanPath(assetsPrefix + "/original_maps/") + "/";
    baseJson = QDir::cleanPath(assetsPrefix + "/matrices/") + "/";

    userMaps = QDir::cleanPath(userMapsPrefix) + "/";
}

void MainWindow::setupMenus() {
    QMenu* mapMenu = menuBar()->addMenu("Seleccionar ciudad");
    mapMenu->addAction("San Andreas", this, [this]() { loadBaseMap("SanAndreas"); });
    mapMenu->addAction("Vice City", this, [this]() { loadBaseMap("ViceCity"); });
    mapMenu->addAction("Liberty City", this, [this]() { loadBaseMap("LibertyCity"); });

    QMenu* archivo = menuBar()->addMenu("Archivo");
    archivo->addAction(&guardarMatrizAction);
    archivo->addAction(&abrirMapaJugableAction);
    archivo->addAction(&guardarMapaJugableAction);
}

void MainWindow::setupActions() {
    guardarMapaJugableAction.setShortcut(QKeySequence("Ctrl+Shift+S"));
    abrirMapaJugableAction.setShortcut(QKeySequence("Ctrl+O"));
    guardarMatrizAction.setShortcut(QKeySequence("Ctrl+S"));
}

void MainWindow::connectActions() {
    connect(&guardarMapaJugableAction, &QAction::triggered, this,
            &MainWindow::onGuardarMapaJugable);

    connect(&abrirMapaJugableAction, &QAction::triggered, this, &MainWindow::onAbrirMapaJugable);
    connect(&guardarMatrizAction, &QAction::triggered, this, &MainWindow::onGuardar);
}

void MainWindow::onGuardar() {
    if (currentMatrixFile.isEmpty()) {
        onGuardarMapaJugable();
        return;
    }

    if (!mapView.isPlayable()) {
        QMessageBox::warning(this, "Mapa incompleto",
                             "El mapa no es jugable.\nDebe tener al menos:\n"
                             "• Una salida\n"
                             "• Una meta\n"
                             "• Un checkpoint");
        return;
    }

    QFileInfo info(currentMatrixFile);
    QString baseName = info.fileName();

    if (protectedMaps.contains(baseName)) {
        QMessageBox::warning(this, "Mapa protegido",
                             "Este es un mapa base original y no puede sobrescribirse.\n"
                             "Usá 'Guardar como...' para crear una copia modificable.");
        return;
    }

    if (!mapView.saveMatrixToJson(currentMatrixFile, mapView.getCurrentMapImageFile())) {
        QMessageBox::warning(this, "Error", "No se pudo guardar el mapa.");
        return;
    }

    QMessageBox::information(this, "Guardado", "Cambios guardados correctamente.");
    statusBar()->showMessage("Guardado: " + info.fileName());
}

void MainWindow::onGuardarMapaJugable() {
    QString filePath = QFileDialog::getSaveFileName(this, "Guardar mapa jugable como...", userMaps,
                                                    "Mapas jugables (*.json)");

    if (filePath.isEmpty())
        return;

    if (!mapView.isPlayable()) {
        QMessageBox::warning(this, "Mapa incompleto",
                             "El mapa no es jugable.\nDebe tener al menos:\n"
                             "• Una salida\n"
                             "• Una meta\n"
                             "• Un checkpoint");
        return;
    }


    if (!filePath.endsWith(".json", Qt::CaseInsensitive))
        filePath += ".json";

    if (QFile::exists(filePath)) {
        if (QMessageBox::question(this, "Reemplazar archivo",
                                  "El archivo ya existe.\n¿Querés sobrescribirlo?",
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
            return;
    }

    if (!mapView.saveMatrixToJson(filePath, mapView.getCurrentMapImageFile())) {
        QMessageBox::warning(this, "Error", "No se pudo guardar el mapa.");
        return;
    }

    currentMatrixFile = filePath;
    currentMapImageFile = mapView.getCurrentMapImageFile();

    QMessageBox::information(this, "Mapa guardado", "Mapa jugable guardado en:\n" + filePath);

    statusBar()->showMessage("Editando mapa jugable: " + QFileInfo(filePath).fileName());
}

void MainWindow::onAbrirMapaJugable() {
    QString filePath = QFileDialog::getOpenFileName(this, "Seleccionar mapa jugable", userMaps,
                                                    "Mapas jugables (*.json)");

    if (filePath.isEmpty())
        return;

    if (!mapView.loadMatrixFromJson(filePath)) {
        QMessageBox::warning(this, "Error", "No se pudo abrir el mapa.");
        return;
    }

    currentMatrixFile = filePath;
    currentMapImageFile = mapView.getCurrentMapImageFile();

    statusBar()->showMessage("Mapa cargado: " + QFileInfo(filePath).fileName());
}

void MainWindow::loadBaseMap(const QString& cityName) {
    if (mapView.isMapModified()) {
        QMessageBox::StandardButton reply =
                QMessageBox::warning(this, "Mapa sin guardar",
                                     "Hay cambios en el mapa que no guardaste.\n"
                                     "¿Seguro que quieres salir sin guardar?",
                                     QMessageBox::Yes | QMessageBox::Cancel);

        if (reply == QMessageBox::Cancel) {
            return;
        }
    }

    QString imageFile = baseMaps + cityName + ".png";
    QString jsonFile = baseJson + "walkable_tiles_" + cityName.toLower() + "_completo.json";

    mapView.loadMap(imageFile);

    if (!mapView.loadMatrixFromJson(jsonFile)) {
        QMessageBox::warning(this, "Error",
                             QString("No se pudo cargar la matriz base:\n%1").arg(jsonFile));
        return;
    }

    currentMapImageFile = imageFile;
    currentMatrixFile = jsonFile;
    statusBar()->showMessage("Mapa base cargado: " + cityName);
}

void MainWindow::createToolsToolbar() {
    QToolBar* tb = addToolBar("Herramientas");

    QActionGroup* grupo = new QActionGroup(this);
    grupo->setExclusive(true);

    salida.setCheckable(true);
    llegada.setCheckable(true);
    checkpoint.setCheckable(true);
    borrar.setCheckable(true);

    tb->addAction(&salida);
    tb->addAction(&llegada);
    tb->addAction(&checkpoint);
    tb->addAction(&borrar);

    QAction* deshacer = tb->addAction("Deshacer");
    connect(deshacer, &QAction::triggered, &mapView, &MapView::undoLastAction);

    grupo->addAction(&salida);
    grupo->addAction(&llegada);
    grupo->addAction(&checkpoint);
    grupo->addAction(&borrar);

    connect(grupo, &QActionGroup::triggered, this, [this](const QAction* a) {
        MapView::Tool tool = MapView::Tool::None;
        QString label = "Modo: ";

        if (a == &salida) {
            tool = MapView::Tool::PlaceStart;
            label += "Salida";
        } else if (a == &llegada) {
            tool = MapView::Tool::PlaceFinish;
            label += "Meta";
        } else if (a == &checkpoint) {
            tool = MapView::Tool::AddCheckpoint;
            label += "Checkpoint";
        } else if (a == &borrar) {
            tool = MapView::Tool::EraseObject;
            label += "Borrar";
        }

        mapView.setTool(tool);
        statusBar()->showMessage(label, 1500);
    });
}


void MainWindow::closeEvent(QCloseEvent* event) {
    if (mapView.isMapModified()) {
        QMessageBox::StandardButton reply =
                QMessageBox::warning(this, "Mapa sin guardar",
                                     "Hay cambios en el mapa que no guardaste.\n"
                                     "¿Seguro que quieres salir sin guardar?",
                                     QMessageBox::Yes | QMessageBox::Cancel);

        if (reply == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }

    event->accept();
}
